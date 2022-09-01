//
// (c) 2020 chiv, all rights reserved
//
#include "terminal_rc.h"
#include "SerialConnRaw.h"
#include "ConnFactory.h"
#include "ProtoFactory.h"
#include "TextCodecsDialog.h"
#include <functional>
#include <stdio.h>

REGISTER_CONN_INSTANCE("Assistant", "vt100", SerialConnRaw);

namespace {
enum LineBreak_ {
    CR,
    LF,
    CRLF
};
}
//
static int _HexFilter(int c)
{
    if (c >= '0' && c <= '9' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f' || c == ' ' || c == '\r' || c == '\n')
        return c;
    else
        return 0;
}
//
SerialConnRaw::SerialConnRaw(std::shared_ptr<SerialIo> io)
    : mRxShouldStop(false)
    , mTxProto(nullptr)
    , mStopUpdateRx(false)
    , mNumBytesTx(0)
    , mNumBytesRx(0)
{
    WantFocus(true);
    //
    this->mIo = io; //!< Important, let this as the first sentence.
    //
    this->SetConnDescription("A common serial tool by chiv, v1.2a");
    //
    this->mRx.SetFrame(FieldFrame());
    this->mTx.SetFrame(FieldFrame());
    this->mVsp.Vert(mRx.SetEditable(false), mTx);
    // The total width of Vsp is 10000, we use 7000 as our default.
    this->mVsp.SetPos(7000);
    // default settings
    this->mLineSz.SetData(16);
    this->mLineSz.SetEditable(false);
    this->mTxPeriod.SetData(100);
    this->mRxBufferSz.SetData(50000);
    this->mTxInterval.SetData(100);
    this->mLineBreaks.Add(LineBreak_::CR, "CR");
    this->mLineBreaks.Add(LineBreak_::LF, "LF");
    this->mLineBreaks.Add(LineBreak_::CRLF, "CRLF");
    this->mLineBreaks.SetIndex(1); //<! default: LF
    this->mRxBytes.SetText("0");
    this->mTxBytes.SetText("0");
    //------------------------------------------------------------------------
    mProtos.Add(t_("None"));
    mProtos.SetIndex(0);
    auto proto_names = ProtoFactory::Inst()->GetSupportedProtoNames();
    for (size_t k = 0; k < proto_names.size(); ++k) {
        this->mProtos.Add(proto_names[k]);
    }
    //------------------------------------------------------------------------
    InstallActions();
    InstallUsrActions();
    //------------------------------------------------------------------------
    // formats
    Upp::CtrlLayout(*this);
}

SerialConnRaw::~SerialConnRaw()
{
    Stop();
    KillTimeCallback(kPeriodicTimerId);
    delete mTxProto;
}

bool SerialConnRaw::Start()
{
    mRxShouldStop = false;
    mRxThr = std::thread([=] { RxProc(); });
    return true;
}

void SerialConnRaw::Stop()
{
    mRxShouldStop = true;
    if (mRxThr.joinable()) {
        mRxThr.join();
    }
}

void SerialConnRaw::InstallUsrActions()
{
    WhenUsrBar = [=](Bar& bar) {
        bar.Add(t_("Text Codec"), terminal::text_codec(), [=]() {
           TextCodecsDialog d(GetCodec()->GetName().c_str());
           int ret = d.Run();
           if (ret == IDOK) {
               this->SetCodec(d.GetCodecName());
               // refresh rx.
               if (!mRxHex.Get()) {
                   UpdateAsTxt();
               }
           }
       }).Help(t_("Select a text codec"));
       bar.Add(t_("Information"), terminal::info(), [=]() {
           PromptOK(this->GetConnDescription());
       });
    };
}

void SerialConnRaw::InstallActions()
{
    this->mLineSz.WhenAction = [=]() {
        if (this->mLineSz.Accept()) {
            if (mRxHex.Get()) {
                UpdateAsHex();
            }
        }
    };
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
        mNumBytesRx = 0; mRxBytes.SetText("0");
    };
    // clear statistics
    this->mBtnClearNumTx.WhenAction = [=]() {
        mNumBytesTx = 0; mTxBytes.SetText("0");
    };
    this->mBtnClearNumRx.WhenAction = [=]() {
        std::lock_guard<std::mutex> _(mRxBufferLock);
        mNumBytesRx = 0; mRxBytes.SetText("0");
    };
    // pause rx
    this->mBtnPauseRx.WhenAction = [&]() {
        if (mRxThr.joinable()) {
            mRxShouldStop = true;
            mRxThr.join();
            mBtnPauseRx.SetLabel(t_("Resume"));
        } else {
            mRxShouldStop = false;
            mRxThr = std::thread([=]() { RxProc(); });
            mBtnPauseRx.SetLabel(t_("Pause"));
        }
    };
    // period
    this->mTxPeriod.WhenAction = [=]() {
        if (mTxPeriod.Get()) {
            mTxInterval.SetReadOnly();
            SetTimeCallback(
                -mTxInterval.GetData().To<int>(), [=]() { Upp::EnterGuiMutex(); OnSend(); Upp::LeaveGuiMutex(); },
                kPeriodicTimerId); // ID-0
        } else {
            mTxInterval.SetEditable(true);
            KillTimeCallback(kPeriodicTimerId);
        }
    };
    // proto
    mProtos.WhenAction = [=]() {
        delete mTxProto;
        mTxProto = ProtoFactory::Inst()->CreateInst(mProtos.GetValue().ToString(), this);
        if (mTxProto) {
            mProtos.Tip(mTxProto->GetDescription().c_str());
        } else {
            mProtos.Tip("");
        }
    };
    this->mTx.WhenBar = [=](Bar& bar) {
        mTx.StdBar(bar);
        if (mTxProto) {
            bar.Sub(mTxProto->GetName().c_str(), [=](Bar& sub) { mTxProto->WhenUsrBar(sub); });
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
        if (k + 1 == b.size()) {
            sprintf(hex_, "%02x", b[k]);
        } else {
            sprintf(hex_, "%02x ", b[k]);
        }
        out += hex_;
    }
    return out;
}
static inline std::string ToHexString_(const std::string& b)
{
    std::string out;
    for (size_t k = 0; k < b.length(); ++k) {
        char hex_[8];
        if (k + 1 == b.size()) {
            sprintf(hex_, "%02x", (unsigned char)b[k]);
        } else {
            sprintf(hex_, "%02x ", (unsigned char)b[k]);
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
                case 'A':
                case 'a':
                    out.push_back(0x07);
                    break;
                case 'B':
                case 'b':
                    out.push_back(0x08);
                    break;
                case 'F':
                case 'f':
                    out.push_back(0x0c);
                    break;
                case 'N':
                case 'n':
                    out.push_back(0x0a);
                    break;
                case 'R':
                case 'r':
                    out.push_back(0x0d);
                    break;
                case 'T':
                case 't':
                    out.push_back(0x09);
                    break;
                case 'V':
                case 'v':
                    out.push_back(0x0b);
                    break;
                case 'E':
                case 'e':
                    out.push_back(0x1b);
                    break;
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
                        if (text[k + 1] >= '0' && text[k + 1] <= '9' && text[k + 2] >= '0' && text[k + 2] <= '9') {
                            // valid oct
                            unsigned char oct = ((text[k] - '0') << 6) | ((text[k + 1] - '0') << 3) | (text[k + 2] - '0');
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
                        if (IsCharInHex(text[k + 1]) && IsCharInHex(text[k + 2])) {
                            // valid hex
                            auto hex = ToHex_(text.substr(k + 1, 2));
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

static inline std::string ReplaceLineBreak_(std::string& text, int lb)
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
    size_t k = 0;
    while (k < text.length()) {
        if (k < text.length() - 1 && text[k] == '\r' && text[k+1] == '\n') { // windows
            out += lb_;
            k += 2;
        } else if (text[k] == '\r') { // mac
            out += lb_;
            k += 1;
        } else if (text[k] == '\n') { // unix
            out += lb_;
            k += 1;
        } else {
            out.push_back(text[k]);
            k += 1;
        }
    }
    return out;
}
//----------------------------------------------------------------------------------------------
void SerialConnRaw::Set_TxInHex()
{
    std::string tx = mTx.GetData().ToString().ToStd();
    std::string refine_tx = mEnableEscape.Get() ? TranslateEscapeChars(tx) : tx;
    std::string tx_codec = GetCodec()->TranscodeFromUTF8((const unsigned char*)refine_tx.data(), refine_tx.length());
    mTx.Set(ToHexString_(tx_codec));
    // use filter to disable invalid chars.
    mTx.SetFilter(_HexFilter);
    //
    mTx.MoveEnd();
}

void SerialConnRaw::Set_TxInTxt()
{
    std::string tx = mTx.GetData().ToString().ToStd();
    std::vector<unsigned char> out = ToHex_(tx);
    mTx.Set(GetCodec()->TranscodeToUTF8(out.data(), out.size()));
    //
    mTx.SetFilter(nullptr);
    //
    mTx.MoveEnd();
}

void SerialConnRaw::OnSend()
{
    int ret = 0;
    std::string errmsg;
    std::string tx = mTx.GetData().ToString().ToStd();
    if (mTxHex.Get()) {
        // write as hex.
        auto hex_ = ToHex_(tx);
        if (mTxProto) {
            if (mTxProto->SupportTransmitData()) {
                ret = mTxProto->TransmitData(hex_.data(), hex_.size(), errmsg);
            } else {
                errmsg = mTxProto->GetName() + " dose not support TransmitData";
            }
        } else {
            ret = (int)GetIo()->Write(hex_);
        }
    } else {
        std::string text = tx;
        if (mEnableEscape.Get()) {
            text = TranslateEscapeChars(text);
        }
        if (mTxProto) {
            if (mTxProto->SupportTransmitData()) {
                ret = mTxProto->TransmitData(text.c_str(), text.length(), errmsg);
            } else {
                errmsg = mTxProto->GetName() + " dose not support TransmitData";
            }
        } else {
            auto tmp = ReplaceLineBreak_(text, mLineBreaks.GetKey(mLineBreaks.GetIndex()).To<int>());
            ret = (int)GetIo()->Write(GetCodec()->TranscodeFromUTF8((const unsigned char*)tmp.c_str(), tmp.length()));
        }
    }
    WhenWarning(errmsg.c_str());
    //
    if (ret > 0)
        mNumBytesTx += ret;
    // update
    mTxBytes.SetText(std::to_string(mNumBytesTx).c_str());
}

void SerialConnRaw::UpdateAsTxt()
{
    const auto& buf = mRxBuffer;
    mRx.Set(GetCodec()->TranscodeToUTF8(buf.data(), buf.size()));
}

void SerialConnRaw::UpdateAsHex()
{
    String text;
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
    //
    mRx.SetData(text);
}

void SerialConnRaw::Update()
{
    if (!mStopUpdateRx) {
        std::lock_guard<std::mutex> _(mRxBufferLock);
        Upp::int64 l, h;
        bool sel = mRx.GetSelection(l, h);
        Point scroll_pos = mRx.GetScrollPos();
        Size pgsz = mRx.GetPageSize();
        bool scroll_to_end = pgsz.cy + scroll_pos.y >= mRx.GetLineCount();
        this->mRxHex.Get() ? UpdateAsHex() : UpdateAsTxt();
        if (sel) {
            mRx.SetSelection(l, h);
        }
        if (scroll_to_end) {
            scroll_pos.x = 0;
            scroll_pos.y = mRx.GetLineCount() - pgsz.cy;
        }
        mRx.SetScrollPos(scroll_pos);
        //
        mRxBytes.SetText(std::to_string(mNumBytesRx).c_str());
    }
    mTxBytes.SetText(std::to_string(mNumBytesTx).c_str());
}

void SerialConnRaw::RxProc()
{
    while (!mRxShouldStop) {
        int sz = GetIo()->Available();
        if (sz < 0) {
            PostCallback([=]() { PromptOK(DeQtf(this->ConnName() + ":" + t_("I/O device was disconnected"))); });
            break;
        } else if (sz == 0) {
            std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
            continue;
        } else {
            size_t max_buffer_sz = mRxBufferSz;
            std::vector<unsigned char> buf = GetIo()->ReadRaw(sz);
            if (mLoopback.Get()) {
                // don't worry, lib serial is thread-safe
                mNumBytesTx += GetIo()->Write(buf);
            }
            mRxBufferLock.lock();
            {
                mRxBuffer.insert(mRxBuffer.end(), buf.begin(), buf.end());
                if (mRxBuffer.size() > max_buffer_sz) {
                    size_t hd = mRxBuffer.size() - max_buffer_sz;
                    mRxBuffer.erase(mRxBuffer.begin(), mRxBuffer.begin() + hd);
                }
                mNumBytesRx += buf.size();
            }
            mRxBufferLock.unlock();
            //
            PostCallback([=]() { Update(); });
        }
    }
}
