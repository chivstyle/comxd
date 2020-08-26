//
// (c) 2020 chiv, all rights reserved
//
#include "terminal_rc.h"
#include "SerialConnRaw.h"
#include "ConnFactory.h"
#include "ProtoFactory.h"
#include <functional>
#include <stdio.h>
//
REGISTER_CONN_INSTANCE("Raw", SerialConnRaw);
//
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
    mProtos.WhenAction = [=]() {
        delete mTxProto;
        mTxProto = ProtoFactory::Inst()->CreateInst(mProtos.GetValue().ToString());
        if (mTxProto) {
            mProtos.Tip(mTxProto->GetDescription().c_str());
        } else {
            mProtos.Tip("");
        }
    };
    //------------------------------------------------------------------------
    InstallActions();
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

void SerialConnRaw::OnSend()
{
    std::string tx = mTx.GetData().ToString().ToStd(); // UTF-8
    if (mTxHex.Get()) {
        // write as hex.
        mSerial->Write(ToHex_(tx));
    } else {
        std::string text = ReplaceLineBreak_(tx, mLineBreaks.GetKey(mLineBreaks.GetIndex()).To<int>());
        if (mEnableEscape.Get()) {
            text = TranslateEscapeChars(text);
        }
        if (mTxProto) {
            std::string errmsg;
            auto out = mTxProto->Pack((unsigned char*)text.c_str(), text.length(), errmsg);
            if (out.empty()) {
                PromptOK(Upp::DeQtf(errmsg.c_str()));
            } else {
                mSerial->Write(out);
            }
        } else {
            mSerial->Write(text);
        }
    }
}

void SerialConnRaw::UpdateAsTxt()
{
    mRxBufferLock.lock();
    String text(mRxBuffer.data(), mRxBuffer.size());
    mRxBufferLock.unlock();
    mRx.SetData(text);
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