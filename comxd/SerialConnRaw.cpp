//
// (c) 2020 chiv, all rights reserved
//
#include "SerialConnRaw.h"
#include <functional>
#include <stdio.h>
//
namespace {
enum LineBreak_ {
    CR,
    LF,
    CRLF
};
}
//
SerialConnRaw::SerialConnRaw(std::shared_ptr<serial::Serial> serial)
    : mRxShouldStop(false)
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
    Ctrl::KillTimeCallback(0);
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
            Ctrl::SetTimeCallback(-mTxInterval.GetData().To<int>(), [=]() { OnSend(); }, 0); // ID-0
        } else {
            mTxInterval.SetEditable(true);
            Ctrl::KillTimeCallback(0);
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
        mSerial->write(ToHex_(tx));
    } else {
        mSerial->write(ReplaceLineBreak_(tx, mLineBreaks.GetKey(mLineBreaks.GetIndex()).To<int>()));
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
        size_t sz = mSerial->available();
        if (sz) {
            size_t max_buffer_sz = mRxBufferSz;
            std::vector<unsigned char> buf;
            mSerial->read(buf, sz);
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
        }
        std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
    }
}
