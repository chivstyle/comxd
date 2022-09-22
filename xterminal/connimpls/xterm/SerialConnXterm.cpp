/*!
// (c) 2021 chiv
//
*/
#include "SerialConnXterm.h"
#include "ConnFactory.h"
#include "XtermCharset.h"
#include "XtermControlSeq.h"

REGISTER_CONN_INSTANCE("xterm", "xterm-256color", SerialConnXterm);

using namespace Upp;

SerialConnXterm::SerialConnXterm(std::shared_ptr<SerialIo> io)
    : SerialConnVT520(io)
    , SerialConnVT(io)
{
    SetConnDescription("xterm emulator, by chiv, v1.1a");
    //
    AddXtermControlSeqs(this->mSeqsFactory);
    //
    LoadDefaultModes();
    //
    InstallFunctions();
}

void SerialConnXterm::LoadDefaultModes()
{
	mModes.SetDecpMode(SendXyOnPress, 0);
	mModes.SetDecpMode(SendXyOnPressAndRelease, 0);
}

void SerialConnXterm::InstallFunctions()
{
    mFunctions[XTPUSHCOLORS] = [=](const std::string& p) { ProcessXTPUSHCOLORS(p); };
    mFunctions[XTPOPCOLORS] = [=](const std::string& p) { ProcessXTPOPCOLORS(p); };
    mFunctions[XTSMGRAPHICS] = [=](const std::string& p) { ProcessXTSMGRAPHICS(p); };
    mFunctions[XTHIMOUSE] = [=](const std::string& p) { ProcessXTHIMOUSE(p); };
    mFunctions[XTRMTITLE] = [=](const std::string& p) { ProcessXTRMTITLE(p); };
    mFunctions[XTMODKEYS] = [=](const std::string& p) { ProcessXTMODKEYS(p); };
    mFunctions[XTSMPOINTER] = [=](const std::string& p) { ProcessXTSMPOINTER(p); };
    mFunctions[XTPUSHSGR] = [=](const std::string& p) { ProcessXTPUSHSGR(p); };
    mFunctions[XTPOPSGR] = [=](const std::string& p) { ProcessXTPOPSGR(p); };
    mFunctions[XTVERSION] = [=](const std::string& p) { ProcessXTVERSION(p); };
    mFunctions[XTRESTORE] = [=](const std::string& p) { ProcessXTRESTORE(p); };
    mFunctions[XTSAVE] = [=](const std::string& p) { ProcessXTSAVE(p); };
    mFunctions[XTWINOPS] = [=](const std::string& p) { ProcessXTWINOPS(p); };
    mFunctions[XTSMTITLE] = [=](const std::string& p) { ProcessXTSMTITLE(p); };
    mFunctions[XTCHECKSUM] = [=](const std::string& p) { ProcessXTCHECKSUM(p); };
    mFunctions[XTDISABLEMODOPTS] = [=](const std::string& p) { ProcessXTDISABLEMODOPTS(p); };
    mFunctions[XTOSC] = [=](const std::string& p) { ProcessXTOSC(p); };
}

void SerialConnXterm::LeftDown(Point p, dword keyflags)
{
    int px = p.x / mFontW + 1, py = p.y / mFontH + 1;
    int cb = 0;
    if (mModes.GetDecpMode(SendXyOnPress, 0) || mModes.GetDecpMode(SendXyOnPressAndRelease, 0)) {
        if (keyflags & K_SHIFT) {
            cb |= 0x04;
        }
        if (keyflags & K_ALT) {
            cb |= 0x08;
        }
        if (keyflags & K_CTRL) {
            cb |= 0x10;
        }
        if (mModes.GetDecpMode(EnableSGRMouseMode, 0)) {
            Put(std::string("\033[<") + std::to_string(cb) + ";" +
                std::to_string(px) + ";" + std::to_string(py) +
                "M");
        } else if (px <= 223 && py <= 223) {
            std::string rsp("\033[M");
            rsp.push_back(32 + cb);
            rsp.push_back(32 + px);
            rsp.push_back(32 + py);
            Put(rsp);
        }
    }
    SerialConnVT520::LeftDown(p, keyflags);
}
void SerialConnXterm::RightDown(Point p, dword keyflags)
{
    int px = p.x / mFontW + 1, py = p.y / mFontH + 1;
    int cb = 1;
    if (mModes.GetDecpMode(SendXyOnPress, 0) || mModes.GetDecpMode(SendXyOnPressAndRelease, 0)) {
        if (keyflags & K_SHIFT) {
            cb |= 0x04;
        }
        if (keyflags & K_ALT) {
            cb |= 0x08;
        }
        if (keyflags & K_CTRL) {
            cb |= 0x10;
        }
        if (mModes.GetDecpMode(EnableSGRMouseMode, 0)) {
            Put(std::string("\033[<") + std::to_string(cb) + ";" +
                std::to_string(px) + ";" + std::to_string(py) +
                "M");
        } else if (px <= 223 && py <= 223) {
            std::string rsp("\033[M");
            rsp.push_back(32 + cb);
            rsp.push_back(32 + px);
            rsp.push_back(32 + py);
            Put(rsp);
        }
    }
    SerialConnVT520::RightDown(p, keyflags);
}
void SerialConnXterm::MiddleDown(Point p, dword keyflags)
{
    int px = p.x / mFontW + 1, py = p.y / mFontH + 1;
    int cb = 2;
    if (mModes.GetDecpMode(SendXyOnPress, 0) || mModes.GetDecpMode(SendXyOnPressAndRelease, 0)) {
        if (keyflags & K_SHIFT) {
            cb |= 0x04;
        }
        if (keyflags & K_ALT) {
            cb |= 0x08;
        }
        if (keyflags & K_CTRL) {
            cb |= 0x10;
        }
        if (mModes.GetDecpMode(EnableSGRMouseMode, 0)) {
            Put(std::string("\033[<") + std::to_string(cb) + ";" +
                std::to_string(px) + ";" + std::to_string(py) +
                "M");
        } else if (px <= 223 && py <= 223) {
            std::string rsp("\033[M");
            rsp.push_back(32 + cb);
            rsp.push_back(32 + px);
            rsp.push_back(32 + py);
            Put(rsp);
        }
    }
    SerialConnVT520::MiddleDown(p, keyflags);
}
void SerialConnXterm::LeftUp(Point p, dword keyflags)
{
    int px = p.x / mFontW + 1, py = p.y / mFontH + 1;
    int cb = 3;
    if (mModes.GetDecpMode(SendXyOnPressAndRelease, 0)) { // X11 mouse tracking
        if (keyflags & K_SHIFT) {
            cb |= 0x04;
        }
        if (keyflags & K_ALT) {
            cb |= 0x08;
        }
        if (keyflags & K_CTRL) {
            cb |= 0x10;
        }
        if (mModes.GetDecpMode(EnableSGRMouseMode, 0)) {
            Put(std::string("\033[<") + std::to_string(cb) + ";" +
                std::to_string(px) + ";" + std::to_string(py) +
                "m");
        } else if (px <= 223 && py <= 223) {
            std::string rsp("\033[M");
            rsp.push_back(32 + cb);
            rsp.push_back(32 + px);
            rsp.push_back(32 + py);
            Put(rsp);
        }
    }
    SerialConnVT520::LeftUp(p, keyflags);
}
void SerialConnXterm::RightUp(Point p, dword keyflags)
{
    int px = p.x / mFontW + 1, py = p.y / mFontH + 1;
    int cb = 3;
    if (mModes.GetDecpMode(SendXyOnPressAndRelease, 0)) { // X11 mouse tracking
        if (keyflags & K_SHIFT) {
            cb |= 0x04;
        }
        if (keyflags & K_ALT) {
            cb |= 0x08;
        }
        if (keyflags & K_CTRL) {
            cb |= 0x10;
        }
        if (mModes.GetDecpMode(EnableSGRMouseMode, 0)) {
            Put(std::string("\033[<") + std::to_string(cb) + ";" +
                std::to_string(px) + ";" + std::to_string(py) +
                "m");
        } else if (px <= 223 && py <= 223) {
            std::string rsp("\033[M");
            rsp.push_back(32 + cb);
            rsp.push_back(32 + px);
            rsp.push_back(32 + py);
            Put(rsp);
        }
    }
    SerialConnVT520::RightUp(p, keyflags);
}
void SerialConnXterm::MiddleUp(Point p, dword keyflags)
{
    int px = p.x / mFontW + 1, py = p.y / mFontH + 1;
    int cb = 3;
    if (mModes.GetDecpMode(SendXyOnPressAndRelease, 0)) { // X11 mouse tracking
        if (keyflags & K_SHIFT) {
            cb |= 0x04;
        }
        if (keyflags & K_ALT) {
            cb |= 0x08;
        }
        if (keyflags & K_CTRL) {
            cb |= 0x10;
        }
        if (mModes.GetDecpMode(EnableSGRMouseMode, 0)) {
            Put(std::string("\033[<") + std::to_string(cb) + ";" +
                std::to_string(px) + ";" + std::to_string(py) +
                "m");
        } else if (px <= 223 && py <= 223) {
            std::string rsp("\033[M");
            rsp.push_back(32 + cb);
            rsp.push_back(32 + px);
            rsp.push_back(32 + py);
            Put(rsp);
        }
    }
    SerialConnVT520::MiddleUp(p, keyflags);
}
void SerialConnXterm::MouseWheel(Upp::Point p, int zdelta, Upp::dword keyflags)
{
    int px = p.x / mFontW + 1, py = p.y / mFontH + 1;
    int cb = zdelta > 0 ? 5 : 4;
    if (mModes.GetDecpMode(SendXyOnPress, 0) || mModes.GetDecpMode(SendXyOnPressAndRelease, 0)) { // X10 mouse
        if (keyflags & K_SHIFT) {
            cb |= 0x04;
        }
        if (keyflags & K_ALT) {
            cb |= 0x08;
        }
        if (keyflags & K_CTRL) {
            cb |= 0x10;
        }
        if (mModes.GetDecpMode(EnableSGRMouseMode)) {
            Put(std::string("\033[<") + std::to_string(cb) + ";" +
                std::to_string(px) + ";" + std::to_string(py) +
                "M");
        } else if (px <= 191 && py <= 191) {
            std::string rsp("\033[M");
            rsp.push_back(64 + cb);
            rsp.push_back(64 + px);
            rsp.push_back(64 + py);
            Put(rsp);
        }
    }
    SerialConnVT520::MouseWheel(p, zdelta, keyflags);
}

void SerialConnXterm::GotFocus()
{
    if (mModes.GetDecpMode(SendFocusInAndOut, 0)) {
        Put("\033[I");
    }
    SerialConnVT520::GotFocus();
}
void SerialConnXterm::LostFocus()
{
    if (mModes.GetDecpMode(SendFocusInAndOut, 0)) {
        Put("\033[O");
    }
    SerialConnVT520::LostFocus();
}

void SerialConnXterm::ProcessSGR(const std::string& p)
{
    std::vector<int> ps;
    SplitString(p.data(), ":;", [=, &ps](const char* token) {
        ps.push_back(atoi(token));
    });
    for (size_t k = 0; k < ps.size(); ++k) {
        // 38:<2/5>:<[Pi]:r:g:b/index>
        if (ps[k] == 38 && k + 1 < ps.size()) { // xterm color
            if (ps[k+1] == 2 && k + 1 + 3 < ps.size()) { // maybe RGB
                if (k+1 + 4 < ps.size()) { // Pi:r:g:b
                    mStyle.FgColorId = this->mColorTbl.FindNearestColorId(Color(ps[k+3], ps[k+4], ps[k+5]));
                    k += 5;
                } else { // r:g:b
                    mStyle.FgColorId = this->mColorTbl.FindNearestColorId(Color(ps[k+2], ps[k+3], ps[k+4]));
                    k += 4;
                }
                // ignore invalid ones
            }
            else if (ps[k+1] == 5 && k + 1 + 1 < ps.size()) { // maybe index color
                mStyle.FgColorId = this->mColorTbl.FindNearestColorId(ps[k+2]);
                k += 2;
            }
            // others were ignored
        } else if (ps[k] == 48 && k < ps.size() - 1) { // xterm color
            if (ps[k+1] == 2 && k + 1 + 3 < ps.size()) { // maybe RGB
                if (k+1 + 4 < ps.size()) { // Pi:r:g:b
                    mStyle.BgColorId = this->mColorTbl.FindNearestColorId(Color(ps[k+3], ps[k+4], ps[k+5]));
                    k += 5;
                } else { // r:g:b
                    mStyle.BgColorId = this->mColorTbl.FindNearestColorId(Color(ps[k+2], ps[k+3], ps[k+4]));
                    k += 4;
                }
                // ignore invalid ones
            }
            else if (ps[k+1] == 5 && k + 1 + 1 < ps.size()) { // maybe index color
                mStyle.BgColorId = this->mColorTbl.FindNearestColorId(ps[k+2]);
                k += 2;
            }
            // others were ignored
        } else {
            UseSGR(ps[k]);
        }
    }
}

void SerialConnXterm::ProcessXTPUSHCOLORS(const std::string&)
{
}
void SerialConnXterm::ProcessXTPOPCOLORS(const std::string&)
{
}
void SerialConnXterm::ProcessXTSMGRAPHICS(const std::string&)
{
}
void SerialConnXterm::ProcessXTHIMOUSE(const std::string&)
{
}
void SerialConnXterm::ProcessXTRMTITLE(const std::string&)
{
}
void SerialConnXterm::ProcessXTDISABLEMODOPTS(const std::string& p)
{
    SplitString(p.data(), ";", [=](const char* token) {
        switch (atoi(token)) {
        case 0: mRcs["modifyKeyboard"] = "0"; break;
        case 1: mRcs["modifyCursorKeys"] = "0"; break;
        case 2: mRcs["modifyFunctionKeys"] = "0"; break;
        case 4: mRcs["modifyOtherKeys"] = "0"; break;
        }
    });
}
void SerialConnXterm::ProcessXTMODKEYS(const std::string& p)
{
    std::string ps[2] = {"", ""};
    int idx = 0;
    SplitString(p.data(), ";", [=, &idx, &ps](const char* token) {
        if (idx < 2)
            ps[idx] = token;
        idx++;
    });
    switch (atoi(ps[0].c_str())) {
    case 0: mRcs["modifyKeyboard"] = ps[1]; break;
    case 1: mRcs["modifyCursorKeys"] = ps[1]; break;
    case 2: mRcs["modifyFunctionKeys"] = ps[1]; break;
    case 4: mRcs["modifyOtherKeys"] = ps[1]; break;
    }
}
void SerialConnXterm::ProcessXTSMPOINTER(const std::string&)
{
    // Ignore this Seq.
}
void SerialConnXterm::ProcessXTPUSHSGR(const std::string&)
{
}
void SerialConnXterm::ProcessXTPOPSGR(const std::string&)
{
}
void SerialConnXterm::ProcessXTVERSION(const std::string&)
{
    Put("\033P|>chiv-xterm v1.0\033\\");
}
void SerialConnXterm::ProcessXTRESTORE(const std::string&)
{
}
void SerialConnXterm::ProcessXTSAVE(const std::string&)
{
}
void SerialConnXterm::ProcessXTWINOPS(const std::string& p)
{
    int ps[3] = {-1, -1, -1};
    int idx = 0;
    SplitString(p.data(), ";", [&idx, &ps](const char* token) {
        if (idx < 3)
            ps[idx++] = atoi(token);
    });
    switch (ps[0]) {
    case 13: if (1) { // report xterm window position, in pixels
        Put("\033[3;0;0t");
    } break;
    case 14: if (1) {
        std::string rsp = "\033[4;" + std::to_string(GetViewSize().cy) + ";" +
            std::to_string(GetViewSize().cx) + "t";
        Put(rsp);
    } break;
    case 15: if (1) { // report size of the screen in pixels
        std::string rsp = "\033[5;" + std::to_string(GetViewSize().cy) + ";" +
            std::to_string(GetViewSize().cx) + "t";
        Put(rsp);
    } break;
    case 16: if (1) { // report xterm character cell size in pixels
        std::string rsp = "\033[6;" + std::to_string(mFontH) + ";" +
            std::to_string(mFontW) + "t";
        Put(rsp);
    } break;
    case 18: if (1) { // Report the size of the text area in characters
        Size csz = GetConsoleSize();
        std::string rsp = "\033[8;" + std::to_string(csz.cy) + ";" +
            std::to_string(csz.cx) + "t";
        Put(rsp);
    } break;
    case 19: if (1) { // Report the size of the screen in characters.
        Size csz = GetConsoleSize();
        std::string rsp = "\033[9;" + std::to_string(csz.cy) + ";" +
            std::to_string(csz.cx) + "t";
        Put(rsp);
    } break;
    case 20: if (1) { // Report xterm window’s icon label
        std::string rsp = "\033]L" + this->ConnName().ToStd() + "\033\\";
        Put(rsp);
    } break;
    case 21: if (1) { // Report xterm window’s title
        std::string rsp = "\033]L" + GetTitle().ToStd() + "\033\\";
        Put(rsp);
    } break;
    // Ignore others, such as "save xterm title", "restore xterm title", .etc
    }
}
void SerialConnXterm::ProcessXTSMTITLE(const std::string&)
{
}
void SerialConnXterm::ProcessXTCHECKSUM(const std::string&)
{
}

void SerialConnXterm::Paste()
{
    CHECK_GUI_THREAD();
    //
    if (mModes.GetDecpMode(SetBracketedPasteMode, 0)) {
        Put("\033[200~");
        SerialConnVT520::Paste();
        Put("\033[201~");
    } else {
        SerialConnVT520::Paste();
    }
}

void SerialConnXterm::ProcessSD(const std::string& p)
{
    // If there are 5 parameters in p, it's a XTHIMOUSE, otherwise it's SD
    int idx = 0;
    SplitString(p.data(), ";", [=, &idx](const char*) { idx++; });
    switch (idx) {
    case 1:
        SerialConnVT520::ProcessSD(p);
        break;
    case 5:
        ProcessXTHIMOUSE(p);
        break;
    default:
        break;
    }
}

void SerialConnXterm::ProcessDECSM(const std::string& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 1048:
        mModes.SetDecpMode(pn, 1);
        this->ProcessDECSC("");
        break;
    case 47:
    case 1047:
    case 1049:
        // switch to alternate screen
        if (!mModes.GetDecpMode(UseAlternateScreen_47, 0) &&
            !mModes.GetDecpMode(UseAlternateScreen_1047, 0) &&
            !mModes.GetDecpMode(SaveCursorAndSwitchToAlternateScreenThenClear, 0)) {
            if (pn == 1049) {
                this->ProcessDECSC("");
            }
            this->SwapScr(mAlternateScr);
            if (pn == 1049) {
                ClearVt();
            }
        }
        mModes.SetDecpMode(pn, 1);
        break;
    default:
        SerialConnVT520::ProcessDECSM(p);
        break;
    }
}

void SerialConnXterm::ProcessDECRM(const std::string& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 1048:
        mModes.SetDecpMode(pn, 0);
        this->ProcessDECRC(p);
        break;
    case 47:
    case 1047:
    case 1049:
        // switch to main screen
        if (mModes.GetDecpMode(UseAlternateScreen_47, 0) ||
            mModes.GetDecpMode(UseAlternateScreen_1047, 0) ||
            mModes.GetDecpMode(SaveCursorAndSwitchToAlternateScreenThenClear, 0)) {
            this->SwapScr(mAlternateScr);
            if (pn == 1049) {
                this->ProcessDECRC("");
            }
            // clear all bits
            mModes.SetDecpMode(UseAlternateScreen_47, 0);
            mModes.SetDecpMode(UseAlternateScreen_1047, 0);
            mModes.SetDecpMode(SaveCursorAndSwitchToAlternateScreenThenClear, 0);
        }
        break;
    default:
        SerialConnVT520::ProcessDECRM(p);
        break;
    }
}
