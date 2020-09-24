//
// (c) 2020 chiv
//
#include "SerialConnXterm.h"
#include "XtermControlSeq.h"
#include "ConnFactory.h"


REGISTER_CONN_INSTANCE("Xterm", SerialConnXterm);

using namespace Upp;

SerialConnXterm::SerialConnXterm(std::shared_ptr<SerialIo> serial)
    : Superclass(serial)
    , SerialConnVT102(serial)
    , SerialConnECMA48(serial)
    , mIsAltScr(false)
{
    //
    this->mPaperColor = Color(10, 10, 10);
    this->mTextsColor = Color(200, 200, 200);
    //
    InstallXtermFunctions();
    // The default setting of alt screen is fixed, inherit from VT.
    // The user can change settings when the alt screen is active, but we do not store the
    // configuration after the user exit the alt screen.
    SaveScr(mAltScr);
}
//
SerialConnXterm::~SerialConnXterm()
{
}

void SerialConnXterm::ProcessXtermTrivial(const std::string& seq)
{
    auto it = mXtermTrivialHandlers.find(seq);
    if (it != mXtermTrivialHandlers.end()) {
        it->second();
    }
}

void SerialConnXterm::ProcessDA(const std::string& seq)
{
    int ps = atoi(seq.substr(1, seq.length()-2).c_str());
    switch (ps) {
    case 0:
        GetSerial()->Write("\x1b[?6c"); // CSI ? 6 c, stands for VT102.
        break;
    }
}

bool SerialConnXterm::ProcessAsciiControlChar(char cc)
{
    return SerialConnECMA48::ProcessAsciiControlChar(cc);
}

bool SerialConnXterm::ProcessControlSeq(const std::string& seq, int seq_type)
{
    if (seq_type == Xterm_Trivial) {
        ProcessXtermTrivial(seq);
    } else if (seq_type > Xterm_Trivial && seq_type < Xterm_SeqType_Endup) {
        auto it = mXtermFuncs.find(seq_type);
        if (it != mXtermFuncs.end()) {
            it->second(seq);
        }
    } else {
        bool processed = SerialConnECMA48::ProcessControlSeq(seq, seq_type);
        if (!processed) {
            processed = SerialConnVT102::ProcessControlSeq(seq, seq_type);
            if (!processed) {
                return Superclass::ProcessControlSeq(seq, seq_type);
            }
        }
    }
    return true;
}

int SerialConnXterm::IsControlSeq(const std::string& seq)
{
    auto seq_type = IsXtermControlSeq(seq);
    if (seq_type == SEQ_NONE) {
        seq_type = SerialConnECMA48::IsControlSeq(seq);
        if (seq_type == SEQ_NONE) {
            seq_type = SerialConnVT102::IsControlSeq(seq);
            if (seq_type == SEQ_NONE) {
                seq_type = Superclass::IsControlSeq(seq);
            }
        }
    }
    return seq_type;
}

bool SerialConnXterm::ProcessKeyDown(Upp::dword key, Upp::dword flags)
{
    std::string d;
    if (flags == 0) {
        switch (key) {
        //--------------------------------------------------------------------------------------
        // VT220 style
        case K_INSERT:
            d = "\033[2~";
            break;
        case K_DELETE:
            d = "\033[3~";
            break;
        case K_HOME:
            d = "\033[1~";
            break;
        case K_END:
            d = "\033[4~";
            break;
        case K_PAGEUP:
            d = "\033[5~";
            break;
        case K_PAGEDOWN:
            d = "\033[6~";
            break;
        //--------------------------------------------------------------------------------------
        // PC Style
        case K_F1:
            d = "\033OP";
            break;
        case K_F2:
            d = "\033OQ";
            break;
        case K_F3:
            d = "\033OR";
            break;
        case K_F4:
            d = "\033OS";
            break;
        case K_F5:
            d = "\033[15~";
            break;
        case K_F6:
            d = "\033[17~";
            break;
        case K_F7:
            d = "\033[18~";
            break;
        case K_F8:
            d = "\033[19~";
            break;
        case K_F9:
            d = "\033[20~";
            break;
        case K_F10:
            d = "\033[21~";
            break;
        case K_F11:
            d = "\033[23~";
            break;
        case K_F12:
            d = "\033[24~";
            break;
        default:break;
        }
    } else {
        int code = 0;
        if ((flags & (K_CTRL | K_SHIFT | K_ALT)) == (K_CTRL | K_SHIFT | K_ALT)) {
            code = 8; // ctrl + alt + shift
        } else if ((flags & (K_CTRL | K_SHIFT | K_ALT)) == K_SHIFT) {
            code = 2;
        } else if ((flags & (K_CTRL | K_SHIFT | K_ALT)) == K_ALT) {
            code = 3;
        } else if ((flags & (K_CTRL | K_SHIFT | K_ALT)) == (K_SHIFT | K_ALT)) {
            code = 4;
        } else if ((flags & (K_CTRL | K_SHIFT | K_ALT)) == K_CTRL) {
            code = 5;
        } else if ((flags & (K_CTRL | K_SHIFT | K_ALT)) == (K_SHIFT | K_CTRL)) {
            code = 6;
        } else if ((flags & (K_CTRL | K_SHIFT | K_ALT)) == (K_ALT | K_CTRL)) {
            code = 7;
        }
        if (code != 0) {
            switch (key) {
            case K_F1:
                d = "\033O;" + std::to_string(code) + "P";
                break;
            case K_F2:
                d = "\033O;" + std::to_string(code) + "Q";
                break;
            case K_F3:
                d = "\033O;" + std::to_string(code) + "R";
                break;
            case K_F4:
                d = "\033O;" + std::to_string(code) + "S";
                break;
            case K_F5:
                d = "\033[15;" + std::to_string(code) + "~";
                break;
            case K_F6:
                d = "\033[17;" + std::to_string(code) + "~";
                break;
            case K_F7:
                d = "\033[18;" + std::to_string(code) + "~";
                break;
            case K_F8:
                d = "\033[19;" + std::to_string(code) + "~";
                break;
            case K_F9:
                d = "\033[20;" + std::to_string(code) + "~";
                break;
            case K_F10:
                d = "\033[21;" + std::to_string(code) + "~";
                break;
            case K_F11:
                d = "\033[23;" + std::to_string(code) + "~";
                break;
            case K_F12:
                d = "\033[24;" + std::to_string(code) + "~";
                break;
            default:break;
            }
        }
    }
    if (!d.empty()) {
        GetSerial()->Write(d);
        return true;
    }
    return SerialConnECMA48::ProcessKeyDown(key, flags);
}

bool SerialConnXterm::ProcessKeyUp(dword key, dword flags)
{
    return SerialConnECMA48::ProcessKeyUp(key, flags);
}

void SerialConnXterm::InstallXtermFunctions()
{
    mXtermTrivialHandlers["[?1049h"] = [=]() { // switch to alternative screen
        if (mIsAltScr == false) {
            SaveScr(mBkgScr); // backup current screen
            LoadScr(mAltScr); // load alternative screen.
            mIsAltScr = true;
        }
    };
    mXtermTrivialHandlers["[?1049l"] = [=]() { // switch to main screen
        if (mIsAltScr) {
            LoadScr(mBkgScr);
            mIsAltScr = false;
        }
    };
    mXtermTrivialHandlers["[m"] = [=]() {
        mCurrAttrFuncs.clear();
        mCurrAttrFuncs.push_back([=]() { SetDefaultStyle(); });
    };
    mXtermTrivialHandlers["[H"] = [=]() {
        mVx = 0;
        mVy = 0;
    };
}
