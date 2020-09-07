//
// (c) 2020 chiv, all rights reserved
//
#include "terminal_rc.h"
#include "SerialConnRaw.h"
#include "ConnFactory.h"
#include "ProtoFactory.h"
#include <functional>
#include <stdio.h>

REGISTER_CONN_INSTANCE("Raw", SerialConnRaw);

namespace {
enum LineBreak_ {
    CR,
    LF,
    CRLF
};
}
//
SerialConnRaw::SerialConnRaw(std::shared_ptr<SerialIo> serial)
    : mRxShouldStop(false)
    , mTxProto(nullptr)
{
    this->mSerial = serial;
    // default settings
    this->mLineSz.SetData(16); this->mLineSz.SetEditable(false);
    this->mTxPeriod.SetData(100);
    this->mRxBufferSz.SetData(50000);
    this->mTxInterval.SetData(100);
    this->mLineBreaks.Add(LineBreak_::CR, "CR");
    this->mLineBreaks.Add(LineBreak_::LF, "LF");
    this->mLineBreaks.Add(LineBreak_::CRLF, "CRLF");
    this->mLineBreaks.SetIndex(1); //<! default: LF
    this->mTxHex.WhenAction = [=]() {
        mEnableEscape.SetEditable(mTxHex.Get() == 0);
        mProtos.SetEditable(mTxHex.Get() == 0);
    };
    this->mEnableEscape.Tip(t_("Hex mode does not support this feature"));
    //------------------------------------------------------------------------
    mProtos.Add(t_("None"));mProtos.SetIndex(0);
    std::vector<std::string> protos = ProtoFactory::Inst()->GetSupportedProtos();
    for (size_t k = 0; k < protos.size(); ++k) {
        this->mProtos.Add(protos[k].c_str());
    }
    //------------------------------------------------------------------------
    InstallActions();
    //------------------------------------------------------------------------
    // start receiving thread
    mRxThr = std::thread([=] { RxProc(); });
    //------------------------------------------------------------------------
    // formats
    Upp::CtrlLayout(*this);
}

SerialConnRaw::~SerialConnRaw()
{
    mRxShouldStop = true;
    if (mRxThr.joinable()) {
        mRxThr.join();
    }
    KillTimeCallback(kPeriodicTimerId);
    delete mTxProto;
}

void SerialConnRaw::InstallActions()
{
    this->mLineSz.WhenAction = [=]() { if (this->mLineSz.Accept()) { if (mRxHex.Get()) { UpdateAsHex(); } } };
    this->mRxBufferSz.WhenAction = [=]() { this->mRxBufferSz.Accept(); };
    this->mTxInterval.WhenAction = [=]() { this->mTxInterval.Accept(); };
    // OnSend
    this->mBtnRun <<= THISBACK(OnSend);
    // Txt/Hex
    this->mRxHex.WhenAction = [=]() {
        mRxHex.Get() ? UpdateAsHex() : UpdateAsTxt();
        mLineSz.SetEditable(mRxHex.Get() != 0);
    };
    // clear rx
    this->mBtnClearRx.WhenAction = [=]() {
        std::lock_guard<std::mutex> _(mRxBufferLock);
        mRxBuffer.clear();
        mRx.Clear();
    };
    // pause rx
    this->mBtnPauseRx.WhenAction = [&]() {
        if (mRxThr.joinable()) {
            mRxShouldStop = true;
            mRxThr.join();
        } else {
            mRxShouldStop = false;
            mRxThr = std::thread([=] { RxProc(); });
        }
    };
    // period
    this->mTxPeriod.WhenAction = [=]() {
        if (mTxPeriod.Get()) {
            mTxInterval.SetReadOnly();
            SetTimeCallback(-mTxInterval.GetData().To<int>(), [=]() { OnSend(); }, kPeriodicTimerId); // ID-0
        } else {
            mTxInterval.SetEditable(true);
            KillTimeCallback(kPeriodicTimerId);
        }
    };
    // proto
    mProtos.WhenAction = [=]() {
        delete mTxProto;
        mTxProto = ProtoFactory::Inst()->CreateInst(mProtos.GetValue().ToString());
        if (mTxProto) {
            mProtos.Tip(mTxProto->GetDescription().c_str());
        } else {
            mProtos.Tip("");
        }
    };
    this->mTx.WhenBar = [=](Bar& bar) {
        mTx.StdBar(bar);
        if (mTxProto) {
            auto actions = mTxProto->GetActions();
            if (!actions.empty()) {
                bar.Separator();
                bar.Sub(t_("Proto actions"), [=](Bar& sub) {
                    for (auto it = actions.begin(); it != actions.end(); ++it) {
                        sub.Add(it->Text, it->Icon, it->Func).Help(it->Help);
                    }
                });
            }
        }
    };
    this->mTxHex.WhenAction = [=]() {
        mTxHex.Get() ? Set_TxInHex() : Set_TxInTxt();
    };
    this->mTxHex.Tip(t_("Will use proto to transcode your input"));
}
//-------------------------------------------------------------------------------
// Help functions
static inline std::vector<byte> ToHex_(const std::string& hex_text)
{
    std::vector<byte> out;
    int sn = 0, hex = 0;
    for (size_t k = 0; k < hex_text.size(); ++k) {
        if (hex_text[k] >= '0' && hex_text[k] <= '9') {
            hex <<= 4;
            hex |= hex_text[k] - '0';
            sn++;
        } else if (hex_text[k] >= 'A' && hex_text[k] <= 'F') {
            hex <<= 4;
            hex |= 10 + (hex_text[k] - 'A');
            sn++;
        } else if (hex_text[k] >= 'a' && hex_text[k] <= 'f') {
            hex <<= 4;
            hex |= 10 + (hex_text[k] - 'a');
            sn++;
        } else { // ignore the invalid chars
            if (sn) { //<! before the invalid char, there's already a valid char, we should record it
                out.push_back((byte)hex);
            }
            // clear flags
            sn = 0;
            hex = -1;
        }
        if (sn == 2) { //<! If there's two successive chars
            out.push_back((byte)hex);
            hex = 0;
            sn = 0;
        }
    }
    if (sn) { //<! There's one left
        out.push_back((byte)hex);
    }
    return out;
}
static inline std::string ToHexString_(const std::vector<unsigned char>& b)
{
    std::string out;
    for (size_t k = 0; k < b.size(); ++k) {
        char hex_[8];
        if (k+1 == b.size()) {
            sprintf(hex_, "%02x", b[k]);
        } else {
            sprintf(hex_, "%02x ", b[k]);
        }
        out += hex_;
    }
    return out;
}

static inline bool IsCharInHex(const char cc)
{
    return (cc >= '0' && cc <= '9' || cc >= 'a' && cc <= 'f' || cc >= 'A' && cc <= 'F');
}

static std::string TranslateEscapeChars(const std::string& text)
{
    std::string out;
    for (size_t k = 0; k < text.size(); ++k) {
        if (text[k] == '\\') { // escape begin
            k++;
            if (k >= text.size()) {
                out.push_back('\\');
                break;
            } else {
                switch (text[k]) {
                case 'a': out.push_back(0x07); break;
                case 'b': out.push_back(0x08); break;
                case 'f': out.push_back(0x0c); break;
                case 'n': out.push_back(0x0a); break;
                case 'r': out.push_back(0x0d); break;
                case 't': out.push_back(0x09); break;
                case 'v': out.push_back(0x0b); break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': // this could be oct, try
                    if (text.size() - k >= 3) {
                        if (text[k+1] >= '0' && text[k+1] <= '9' &&
                            text[k+2] >= '0' && text[k+2] <= '9')
                        {
                            // valid oct
                            unsigned char oct = ((text[k] - '0') << 6) |
                                                ((text[k+1] - '0') << 3) |
                                                (text[k+2] - '0');
                            out.push_back((char)oct);
                            k += 2;
                            break;
                        }
                    }
                    // bad, strip slash
                    out.push_back(text[k]);
                    break;
                case 'x': // this could be hex, try
                    if (text.size() - k >= 3) {
                        if (IsCharInHex(text[k+1]) && IsCharInHex(text[k+2])) {
                            // valid hex
                            auto hex = ToHex_(text.substr(k+1, 2));
                            out.push_back((char)hex[0]);
                            k += 2;
                            break;
                        }
                    }
                    // bad, strip slash
                    out.push_back(text[k]);
                    break;
                default:
                    out.push_back(text[k]);
                    break;
                }
            }
        } else {
            out.push_back(text[k]);
        }
    }
    return out;
}

static std::string ReplaceLineBreak_(const std::string& text, int lb)
{
    std::string lb_ = "\r\n";
    switch (lb) {
    case LineBreak_::CR:
        lb_ = "\r";
        break;
    case LineBreak_::LF:
        lb_ = "\n";
        break;
    default:
        break; // CRLF
    }
    std::string out;
    bool lb_u = false; // line break was not added
    for (size_t k = 0; k < text.size(); ++k) {
        if ((text[k] == '\r' || text[k] == '\n') && lb_u == false) {
            out += lb_;
            lb_u = true;
        } else if (text[k] != '\r' && text[k] != '\n') {
            lb_u = false;
            out.push_back(text[k]);
        }
    }
    return out;
}

void SerialConnRaw::Set_TxInHex()
{
    if (mTxProto) {
        std::string tx = mTx.GetData().ToString().ToStd();
        std::string errmsg;
        auto out = mTxProto->Pack(tx, errmsg);
        if (!out.empty()) {
            mTx.Set(ToHexString_(out));
        }
        this->mTmpStatus.SetText(errmsg.c_str());
        mTx.MoveEnd();
    }
}

void SerialConnRaw::Set_TxInTxt()
{
    if (mTxProto) {
        std::string tx = mTx.GetData().ToString().ToStd();
        std::vector<unsigned char> out = ToHex_(tx);
        std::string errmsg;
        std::string json = mTxProto->Unpack(out, errmsg);
        if (!json.empty()) {
            mTx.Set(json);
        }
        this->mTmpStatus.SetText(errmsg.c_str());
        mTx.MoveEnd();
    }
}

void SerialConnRaw::OnSend()
{
    std::string tx = mTx.GetData().ToString().ToStd();
    if (mTxHex.Get()) {
        // write as hex.
        mSerial->Write(ToHex_(tx));
    } else {
        std::string text = tx;
        if (mEnableEscape.Get()) {
            text = TranslateEscapeChars(text);
        }
        if (mTxProto) {
            std::string errmsg;
            auto out = mTxProto->Pack(text, errmsg);
            if (out.empty()) {
                PromptOK(Upp::DeQtf(errmsg.c_str()));
            } else {
                mSerial->Write(out);
            }
        } else {
            mSerial->Write(ReplaceLineBreak_(text, mLineBreaks.GetKey(mLineBreaks.GetIndex()).To<int>()));
        }
    }
}

void SerialConnRaw::UpdateAsTxt()
{
    std::string text;
    mRxBufferLock.lock();
    const auto& buf = mRxBuffer;
    // abc, you known
    if (mShowInvisibleChars.Get()) {
        for (size_t k = 0; k < buf.size(); ++k) {
            if (buf[k] == 0x7f || buf[k] >= 0 && buf[k] < 0x20) {
                switch (buf[k]) {
                case 0x00: text += "<NUL>"; break;
                case 0x01: text += "<SOH>"; break;
                case 0x02: text += "<STX>"; break;
                case 0x03: text += "<ETX>"; break;
                case 0x04: text += "<EOT>"; break;
                case 0x05: text += "<ENQ>"; break;
                case 0x06: text += "<ACK>"; break;
                case 0x07: text += "<BEL>"; break;
                case 0x08: text += "<BS>"; break;
                // case 0x09: break; Tab
                // case 0x0a: break; Line feed
                case 0x0b: text += "<VT>"; break;
                case 0x0c: text += "<FF>"; break;
                // case 0x0d: break; carrier
                case 0x0e: text += "<SO>"; break;
                case 0x0f: text += "<SI>"; break;
                case 0x10: text += "<DLE>"; break;
                case 0x11: text += "<DC1>"; break;
                case 0x12: text += "<DC2>"; break;
                case 0x13: text += "<DC3>"; break;
                case 0x14: text += "<DC4>"; break;
                case 0x15: text += "<NAK>"; break;
                case 0x16: text += "<SYN>"; break;
                case 0x17: text += "<ETB>"; break;
                case 0x18: text += "<CAN>"; break;
                case 0x19: text += "<EM>"; break;
                case 0x1a: text += "<SUB>"; break;
                case 0x1b: text += "<ESC>"; break;
                case 0x1c: text += "<FS>"; break;
                case 0x1d: text += "<GS>"; break;
                case 0x1e: text += "<RS>"; break;
                case 0x1f: text += "<US>"; break;
                case 0x7f: text += "<DEL>"; break;
                default:
                    text.push_back((char)buf[k]);
                    break;
                }
            } else if (buf[k] >= 0x20 && buf[k] < 0x7f) {
                text.push_back((char)buf[k]);
            } else {
                char hex[8];
                sprintf(hex, "\\x%02x", buf[k]);
                text += hex;
            }
        }
    } else {
        text.resize(buf.size());
        for (size_t k = 0; k < buf.size(); ++k) {
            text.push_back((char)buf[k]);
        }
    }
    mRxBufferLock.unlock();
    //
    mRx.Set(text);
    mRx.MoveEnd();
    mRx.ScrollEnd();
}

void SerialConnRaw::UpdateAsHex()
{
    String text;
    mRxBufferLock.lock();
    {
        auto& buf = mRxBuffer;
        int linesz = mLineSz;
        for (size_t k = 0; k < buf.size(); ++k) {
            char buff[8];
            snprintf(buff, 8, "%02X ", buf[k]);
            text += buff;
            if ((k + 1) % linesz == 0) {
                text += "\n";
            }
        }
    }
    mRxBufferLock.unlock();
    //
    mRx.SetData(text);
    mRx.MoveEnd();
    mRx.ScrollEnd();
}

void SerialConnRaw::RxProc()
{
    while (!mRxShouldStop) {
        size_t sz = mSerial->Available();
        if (sz) {
            size_t max_buffer_sz = mRxBufferSz;
            std::vector<unsigned char> buf = mSerial->ReadRaw(sz);
            mRxBufferLock.lock();
            {
                mRxBuffer.insert(mRxBuffer.end(), buf.begin(), buf.end());
                if (mRxBuffer.size() > max_buffer_sz) {
                    size_t hd = mRxBuffer.size() - max_buffer_sz;
                    mRxBuffer.erase(mRxBuffer.begin(), mRxBuffer.begin() + hd);
                }
            }
            mRxBufferLock.unlock();
            //
            Upp::PostCallback([=]() { mRxHex.Get() ? UpdateAsHex() : UpdateAsTxt(); });
        } else std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
    }
}
