/*!
// (c) 2021 chiv
//
*/
#include "SerialConnXterm.h"
#include "ConnFactory.h"
#include "XtermCharset.h"
#include "XtermControlSeq.h"

REGISTER_CONN_INSTANCE("xterm by chiv", "xterm", SerialConnXterm);

using namespace Upp;

SerialConnXterm::SerialConnXterm(std::shared_ptr<SerialIo> io)
    : SerialConnVT520(io)
    , SerialConnVT(io)
{
    AddXtermControlSeqs(this->mSeqsFactory);
    // take a snap the current screen
    this->SaveScr(mAlternateScr);
    //
    InstallFunctions();
}

void SerialConnXterm::InstallFunctions()
{
    mFunctions[XTPUSHCOLORS] = [=](const std::string_view& p) { ProcessXTPUSHCOLORS(p); };
    mFunctions[XTPOPCOLORS] = [=](const std::string_view& p) { ProcessXTPOPCOLORS(p); };
    mFunctions[XTSMGRAPHICS] = [=](const std::string_view& p) { ProcessXTSMGRAPHICS(p); };
    mFunctions[XTHIMOUSE] = [=](const std::string_view& p) { ProcessXTHIMOUSE(p); };
    mFunctions[XTRMTITLE] = [=](const std::string_view& p) { ProcessXTRMTITLE(p); };
    mFunctions[XTMODKEYS] = [=](const std::string_view& p) { ProcessXTMODKEYS(p); };
    mFunctions[XTSMPOINTER] = [=](const std::string_view& p) { ProcessXTSMPOINTER(p); };
    mFunctions[XTPUSHSGR] = [=](const std::string_view& p) { ProcessXTPUSHSGR(p); };
    mFunctions[XTPOPSGR] = [=](const std::string_view& p) { ProcessXTPOPSGR(p); };
    mFunctions[XTVERSION] = [=](const std::string_view& p) { ProcessXTVERSION(p); };
    mFunctions[XTRESTORE] = [=](const std::string_view& p) { ProcessXTRESTORE(p); };
    mFunctions[XTSAVE] = [=](const std::string_view& p) { ProcessXTSAVE(p); };
    mFunctions[XTWINOPS] = [=](const std::string_view& p) { ProcessXTWINOPS(p); };
    mFunctions[XTSMTITLE] = [=](const std::string_view& p) { ProcessXTSMTITLE(p); };
    mFunctions[XTCHECKSUM] = [=](const std::string_view& p) { ProcessXTCHECKSUM(p); };
}

void SerialConnXterm::LeftDown(Point p, dword keyflags)
{
    
    SerialConnVT520::LeftDown(p, keyflags);
}

void SerialConnXterm::ProcessSGR(const std::string_view& p)
{
    std::vector<int> ps;
    SplitString(p.data(), ":;", [=, &ps](const char* token) {
        ps.push_back(atoi(token));
    });
    if (ps.size() <= 2) {
        SerialConnVT520::ProcessSGR(p);
    } else if (ps.size() > 2) {
        switch (ps[0]) {
        case 38:
            if (ps[1] == 2) { // RGB Color
                switch (ps.size()) {
                case 6:
                    // ps[2] is color space, we ignore it.
                    mStyle.FgColorId = this->mColorTbl.FindNearestColorId(Color(ps[3], ps[4], ps[5]));
                    break;
                case 5:
                    mStyle.FgColorId = this->mColorTbl.FindNearestColorId(Color(ps[2], ps[3], ps[4]));
                    break;
                default:break;
                }
            } else if (ps[1] == 5) { // Indexed Color
                mStyle.FgColorId = this->mColorTbl.FindNearestColorId(ps[2]);
            }
            break;
        case 48:
            if (ps[1] == 2) { // RGB Color
                switch (ps.size()) {
                case 6:
                    // ps[2] is color space, we ignore it.
                    mStyle.BgColorId = this->mColorTbl.FindNearestColorId(Color(ps[3], ps[4], ps[5]));
                    break;
                case 5:
                    mStyle.BgColorId = this->mColorTbl.FindNearestColorId(Color(ps[2], ps[3], ps[4]));
                    break;
                default:break;
                }
            } else if (ps[1] == 5) { // Indexed Color
                mStyle.BgColorId = this->mColorTbl.FindNearestColorId(ps[2]);
            }
            break;
        default: break;
        }
    }
}

void SerialConnXterm::ProcessXTPUSHCOLORS(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTPOPCOLORS(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTSMGRAPHICS(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTHIMOUSE(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTRMTITLE(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTMODKEYS(const std::string_view& p)
{
}
void SerialConnXterm::ProcessXTSMPOINTER(const std::string_view&)
{
    // Ignore this Seq.
}
void SerialConnXterm::ProcessXTPUSHSGR(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTPOPSGR(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTVERSION(const std::string_view&)
{
    Put("\EP|>chiv-xterm v1.0\E\\");
}
void SerialConnXterm::ProcessXTRESTORE(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTSAVE(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTWINOPS(const std::string_view& p)
{
    int ps[3] = {-1, -1, -1};
    int idx = 0;
    SplitString(p.data(), ";", [&idx, &ps](const char* token) {
        if (idx < 3)
            ps[idx++] = atoi(token);
    });
    switch (ps[0]) {
    case 13: if (1) { // report xterm window position, in pixels
        Put("\E[3;0;0t");
    } break;
    case 14: if (1) {
        std::string rsp = "\E[4;" + std::to_string(GetSize().cy) + ";" +
            std::to_string(GetSize().cx) + "t";
        Put(rsp);
    } break;
    case 15: if (1) { // report size of the screen in pixels
        std::string rsp = "\E[5;" + std::to_string(GetSize().cy) + ";" +
            std::to_string(GetSize().cx) + "t";
        Put(rsp);
    } break;
    case 16: if (1) { // report xterm character cell size in pixels
        std::string rsp = "\E[6;" + std::to_string(mFontH) + ";" +
            std::to_string(mFontW) + "t";
        Put(rsp);
    } break;
    case 18: if (1) { // Report the size of the text area in characters
        Size csz = GetConsoleSize();
        std::string rsp = "\E[8;" + std::to_string(csz.cy) + ";" +
            std::to_string(csz.cx) + "t";
        Put(rsp);
    } break;
    case 19: if (1) { // Report the size of the screen in characters.
        Size csz = GetConsoleSize();
        std::string rsp = "\E[9;" + std::to_string(csz.cy) + ";" +
            std::to_string(csz.cx) + "t";
        Put(rsp);
    } break;
    case 20: if (1) { // Report xterm window’s icon label
        std::string rsp = "\E]L" + this->Name().ToStd() + "\E\\";
        Put(rsp);
    } break;
    case 21: if (1) { // Report xterm window’s title
        std::string rsp = "\E]L" + this->Name().ToStd() + "\E\\";
        Put(rsp);
    } break;
    // Ignore others, such as "save xterm title", "restore xterm title", .etc
    }
}
void SerialConnXterm::ProcessXTSMTITLE(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTCHECKSUM(const std::string_view&)
{
}

void SerialConnXterm::Paste()
{
    if (mModes.BracketedPaste) {
        Put("\E[200~");
        SerialConnVT520::Paste();
        Put("\E[201~");
    } else {
        SerialConnVT520::Paste();
    }
}

void SerialConnXterm::ProcessSD(const std::string_view& p)
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

void SerialConnXterm::ProcessDECSM(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 9:
        mModes.SendXyOnPress = 1;
        break;
    case 10:
        mModes.ShowToolbar = 1;
        break;
    case 12:
    case 13:
        mModes.BlinkingCursor = 1;
        break;
    case 14:
        mModes.XOR = 1;
        break;
    case 30:
        mModes.ShowScrollbar = 1;
        break;
    case 35:
        mModes.FontShifting = 1;
        break;
    case 40:
        mModes.Allow80T132 = 1;
        break;
    case 41:
        mModes.MoreFix = 1;
        break;
    case 44:
        mModes.MarginBell = 1;
        break;
    case 45:
        mModes.ReverseWrapAround = 1;
        break;
    case 46:
        mModes.Logging = 1;
        break;
    case 1000:
        mModes.SendXyOnPress = 1;
        break;
    case 1001:
        mModes.HiliteMouseTracking = 1;
        break;
    case 1002:
        mModes.CellMotionMouseTracking = 1;
        break;
    case 1003:
        mModes.AllMotionMouseTracking = 1;
        break;
    case 1004:
        mModes.SendFocusInAndOut = 1;
        break;
    case 1005:
        mModes.Utf8Mouse = 1;
        break;
    case 1006:
        mModes.SgrMouse = 1;
        break;
    case 1007:
        mModes.AlternateScroll = 1;
        break;
    case 1010:
        mModes.ScrollToBottomOnOutput = 1;
        break;
    case 1011:
        mModes.ScrollToBottomOnKeyPress = 1;
        break;
    case 1015:
        mModes.UrxVtMouse = 1;
        break;
    case 1034:
        mModes.InterpretMetaKey = 1;
        break;
    case 1035:
        mModes.SpecModAltAndNumlck = 1;
        break;
    case 1036:
        mModes.SendEscOnMetaModifiesKey = 1;
        break;
    case 1037:
        mModes.SendDelFromKeypadDel = 1;
        break;
    case 1039:
        mModes.SendEscOnAltModifiesKey = 1;
        break;
    case 1040:
        mModes.KeepSelection = 1;
        break;
    case 1041:
        mModes.UseClipboardSelection = 1;
        break;
    case 1042:
        mModes.EnableUrgencyWmHint = 1;
        break;
    case 1043:
        mModes.EnableWindowRaising = 1;
        break;
    case 1044:
        // Reuse the most recent data copied to clipboard
        break;
    case 1046:
        mModes.EnableAlternateScr = 1;
        break;
    case 1048:
        SerialConnVT520::ProcessDECSC(p);
        break;
    case 47:
    case 1047:
    case 1049:
        // switch to alternate screen
        if (mModes.UseAlternateScr == 0) {
            this->SwapScr(mAlternateScr);
            mModes.UseAlternateScr = 1;
            if (pn == 1049) {
                Clear();
            }
        }
        break;
    default:
        SerialConnVT520::ProcessDECSM(p);
        break;
    }
}

void SerialConnXterm::ProcessDECRM(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 9:
        mModes.SendXyOnPress = 0;
        break;
    case 10:
        mModes.ShowToolbar = 0;
        break;
    case 12:
    case 13:
        mModes.BlinkingCursor = 0;
        break;
    case 14:
        mModes.XOR = 0;
        break;
    case 30:
        mModes.ShowScrollbar = 0;
        break;
    case 35:
        mModes.FontShifting = 0;
        break;
    case 40:
        mModes.Allow80T132 = 0;
        break;
    case 41:
        mModes.MoreFix = 0;
        break;
    case 44:
        mModes.MarginBell = 0;
        break;
    case 45:
        mModes.ReverseWrapAround = 0;
        break;
    case 46:
        mModes.Logging = 0;
        break;
    case 1000:
        mModes.SendXyOnPress = 0;
        break;
    case 1001:
        mModes.HiliteMouseTracking = 0;
        break;
    case 1002:
        mModes.CellMotionMouseTracking = 0;
        break;
    case 1003:
        mModes.AllMotionMouseTracking = 0;
        break;
    case 1004:
        mModes.SendFocusInAndOut = 0;
        break;
    case 1005:
        mModes.Utf8Mouse = 0;
        break;
    case 1006:
        mModes.SgrMouse = 0;
        break;
    case 1007:
        mModes.AlternateScroll = 0;
        break;
    case 1010:
        mModes.ScrollToBottomOnOutput = 0;
        break;
    case 1011:
        mModes.ScrollToBottomOnKeyPress = 0;
        break;
    case 1015:
        mModes.UrxVtMouse = 0;
        break;
    case 1034:
        mModes.InterpretMetaKey = 0;
        break;
    case 1035:
        mModes.SpecModAltAndNumlck = 0;
        break;
    case 1036:
        mModes.SendEscOnMetaModifiesKey = 0;
        break;
    case 1037:
        mModes.SendDelFromKeypadDel = 0;
        break;
    case 1039:
        mModes.SendEscOnAltModifiesKey = 0;
        break;
    case 1040:
        mModes.KeepSelection = 0;
        break;
    case 1041:
        mModes.UseClipboardSelection = 0;
        break;
    case 1042:
        mModes.EnableUrgencyWmHint = 0;
        break;
    case 1043:
        mModes.EnableWindowRaising = 0;
        break;
    case 1046:
        mModes.EnableAlternateScr = 0;
        break;
    case 1048:
        SerialConnVT520::ProcessDECRC(p);
        break;
    case 47:
    case 1047:
    case 1049:
        // switch to main screen
        if (mModes.UseAlternateScr == 1) {
            this->SwapScr(mAlternateScr);
            mModes.UseAlternateScr = 0;
        }
        break;
    default:
        SerialConnVT520::ProcessDECRM(p);
        break;
    }
}
