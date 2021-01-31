/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT100.h"
#include "VT100ControlSeq.h"
#include "VT100Charset.h"
#include "ConnFactory.h"

using namespace Upp;

REGISTER_CONN_INSTANCE("vt100 by chiv", "vt100", SerialConnVT100);

SerialConnVT100::SerialConnVT100(std::shared_ptr<SerialIo> io)
    : SerialConnVT(io)
    , SerialConnEcma48(io)
{
    AddVT100ControlSeqs(this->mSeqsFactory);
    // VT100, permanently selected modes
    SerialConnEcma48::mModes.CRM = 0;
    SerialConnEcma48::mModes.EBM = 0;
    SerialConnEcma48::mModes.ERM = 1;
    SerialConnEcma48::mModes.FEAM = 0;
    SerialConnEcma48::mModes.PUM = 0;
    SerialConnEcma48::mModes.SRTM = 0;
    SerialConnEcma48::mModes.TSM = 0;
    // enable wrap line
    WrapLine(true);
    // default charsets
    mCharsets[0] = CS_US;
    mCharsets[1] = CS_UK;
    //
    SaveCursor(mCursorData);
    //
    InstallFunctions();
}

void SerialConnVT100::InstallFunctions()
{
    mFunctions[VT100_MODE_SET] = [=](const std::string& p) { ProcessVT100_MODE_SET(p); };
    mFunctions[VT100_MODE_RESET] = [=](const std::string& p) { ProcessVT100_MODE_RESET(p); };
    mFunctions[VT100_DSR] = [=](const std::string& p) { ProcessVT100_DSR(p); };
    //
    mFunctions[VT100_G0_UK]           = [=](const std::string& p) { ProcessVT100_G0_UK(p); };
    mFunctions[VT100_G0_US]           = [=](const std::string& p) { ProcessVT100_G0_US(p); };
    mFunctions[VT100_G0_LINE_DRAWING] = [=](const std::string& p) { ProcessVT100_G0_LINE_DRAWING(p); };
    mFunctions[VT100_G0_ROM]          = [=](const std::string& p) { ProcessVT100_G0_ROM(p); };
    mFunctions[VT100_G0_ROM_SPECIAL]  = [=](const std::string& p) { ProcessVT100_G0_ROM_SPECIAL(p); };
    mFunctions[VT100_G1_UK]           = [=](const std::string& p) { ProcessVT100_G1_UK(p); };
    mFunctions[VT100_G1_US]           = [=](const std::string& p) { ProcessVT100_G1_US(p); };
    mFunctions[VT100_G1_LINE_DRAWING] = [=](const std::string& p) { ProcessVT100_G1_LINE_DRAWING(p); };
    mFunctions[VT100_G1_ROM]          = [=](const std::string& p) { ProcessVT100_G1_ROM(p); };
    mFunctions[VT100_G1_ROM_SPECIAL]  = [=](const std::string& p) { ProcessVT100_G1_ROM_SPECIAL(p); };
    //
    mFunctions[DECREQTPARM] = [=](const std::string& p) { ProcessDECREQTPARM(p); };
    mFunctions[DECSTBM] = [=](const std::string& p) { ProcessDECSTBM(p); };
    mFunctions[DECSC] = [=](const std::string& p) { ProcessDECSC(p); };
    mFunctions[DECRC] = [=](const std::string& p) { ProcessDECRC(p); };
    mFunctions[DECALN] = [=](const std::string& p) { ProcessDECALN(p); };
    mFunctions[DECTST] = [=](const std::string& p) { ProcessDECTST(p); };
    mFunctions[DECLL] = [=](const std::string& p) { ProcessDECLL(p); };
}

void SerialConnVT100::ProcessDA(const std::string& p)
{
    int ps = atoi(p.c_str());
    switch (ps) {
    case 0:
        // https://www.vt100.net/docs/vt100-ug/chapter3.html#DA
        GetIo()->Write("\x1b[?1;2c"); // Advanced video option
        break;
    }
}
//
void SerialConnVT100::ProcessVT100_G0_UK(const std::string& p)
{
    mCharsets[0] = CS_UK;
}
void SerialConnVT100::ProcessVT100_G1_UK(const std::string& p)
{
    mCharsets[1] = CS_UK;
}
void SerialConnVT100::ProcessVT100_G0_US(const std::string& p)
{
    mCharsets[0] = CS_US;
}
void SerialConnVT100::ProcessVT100_G1_US(const std::string& p)
{
    mCharsets[1] = CS_US;
}
void SerialConnVT100::ProcessVT100_G0_LINE_DRAWING(const std::string& p)
{
    mCharsets[0] = CS_LINE_DRAWING;
}
void SerialConnVT100::ProcessVT100_G1_LINE_DRAWING(const std::string& p)
{
    mCharsets[1] = CS_LINE_DRAWING;
}
void SerialConnVT100::ProcessVT100_G0_ROM(const std::string& p)
{
    mCharsets[0] = CS_ROM;
}
void SerialConnVT100::ProcessVT100_G1_ROM(const std::string& p)
{
    mCharsets[1] = CS_ROM;
}
void SerialConnVT100::ProcessVT100_G0_ROM_SPECIAL(const std::string& p)
{
    mCharsets[0] = CS_ROM_SPECIAL;
}
void SerialConnVT100::ProcessVT100_G1_ROM_SPECIAL(const std::string& p)
{
    mCharsets[1] = CS_ROM_SPECIAL;
}
void SerialConnVT100::ProcessLS0(const std::string& p)
{
    mCharset = mCharsets[0];
}
void SerialConnVT100::ProcessLS1(const std::string& p)
{
    mCharset = mCharsets[1];
}
uint32_t SerialConnVT100::RemapCharacter(uint32_t uc, int charset)
{
    if (uc >= ' ' && uc < 0x7f) {
        return VT100_RemapCharacter(uc, charset);
    }
    return SerialConnEcma48::RemapCharacter(uc);
}
//
bool SerialConnVT100::ProcessKeyDown(Upp::dword key, Upp::dword flags)
{
    bool processed = false;
    if (flags == 0) {
        processed = true;
        switch (key) {
        case K_UP: if (1) {
            if (mModes.DECCKM == VT100Modes::DECCKM_Cursor) {
                GetIo()->Write("\033[A");
            } else {
                GetIo()->Write("\033OA");
            }
        } break;
        case K_DOWN:if (1) {
            if (mModes.DECCKM == VT100Modes::DECCKM_Cursor) {
                GetIo()->Write("\033[B");
            } else {
                GetIo()->Write("\033OB");
            }
        } break;
        case K_LEFT:if (1) {
            if (mModes.DECCKM == VT100Modes::DECCKM_Cursor) {
                GetIo()->Write("\033[D");
            } else {
                GetIo()->Write("\033OD");
            }
        } break;
        case K_RIGHT:if (1) {
            if (mModes.DECCKM == VT100Modes::DECCKM_Cursor) {
                GetIo()->Write("\033[C");
            } else {
                GetIo()->Write("\033OC");
            }
        } break;
        case K_F1:  GetIo()->Write("\x1bOP"); break;
		case K_F2:  GetIo()->Write("\x1bOQ"); break;
		case K_F3:  GetIo()->Write("\x1bOR"); break;
		case K_F4:  GetIo()->Write("\x1bOS"); break;
		case K_F5:  GetIo()->Write("\x1bOt"); break;
		case K_F6:  GetIo()->Write("\x1bOu"); break;
		case K_F7:  GetIo()->Write("\x1bOv"); break;
		case K_F8:  GetIo()->Write("\x1bOl"); break;
		case K_F9:  GetIo()->Write("\x1bOw"); break;
		case K_F10: GetIo()->Write("\x1bOx"); break;
        case K_HOME:if (1) {
                GetIo()->Write("\033[H");
        } break;
        default:
            processed = false;
            break;
        }
    }
    return processed ? true : SerialConnEcma48::ProcessKeyDown(key, flags);
}
//
void SerialConnVT100::ProcessVT100_MODE_SET(const std::string& p)
{
    int ps = atoi(p.c_str());
    switch (ps) {
    case 1:  mModes.DECCKM  = 1; break;
    case 3:  mModes.DECCOLM = 1; break;
    case 4:  mModes.DECSCLM = 1; break;
    case 5:
        mModes.DECSCNM = 1; // white screen background with black characters
        mColorTbl.SetColor(VTColorTable::kColorId_Paper, mColorTbl.GetColor(VTColorTable::kColorId_White));
        mColorTbl.SetColor(VTColorTable::kColorId_Texts, mColorTbl.GetColor(VTColorTable::kColorId_Black));
        break;
    case 6:  mModes.DECOM   = 1; ProcessCUP(""); /*! home */ break;
    case 7:  mModes.DECAWM  = 1; WrapLine(true); break;
    case 8:  mModes.DECARM  = 1; break;
    case 18: mModes.DECPFF  = 1; break;
    case 19: mModes.DECPEX  = 1; break;
    }
}
void SerialConnVT100::ProcessVT100_MODE_RESET(const std::string& p)
{
    int ps = atoi(p.c_str());
    switch (ps) {
    case 1:  mModes.DECCKM  = 0; break;
    case 2:  mModes.DECANM  = 0; break;
    case 3:  mModes.DECCOLM = 0; break;
    case 4:  mModes.DECSCLM = 0; break;
    case 5:
        mModes.DECSCNM = 0;
        mColorTbl.SetColor(VTColorTable::kColorId_Paper, mColorTbl.GetColor(VTColorTable::kColorId_Black));
        mColorTbl.SetColor(VTColorTable::kColorId_Texts, mColorTbl.GetColor(VTColorTable::kColorId_White));
        break;
    case 6:  mModes.DECOM   = 0; ProcessCUP(""); /*! home */ break;
    case 7:  mModes.DECAWM  = 0; WrapLine(false); break;
    case 8:  mModes.DECARM  = 0; break;
    case 18: mModes.DECPFF  = 0; break;
    case 19: mModes.DECPEX  = 0; break;
    }
}

void SerialConnVT100::ProcessVT100_DSR(const std::string& p)
{
    int ps = atoi(p.c_str());
    switch (ps) {
    case 15:
        GetIo()->Write("\x1b[?13n"); // No printer
        break;
    }
}

void SerialConnVT100::ProcessCUP(const std::string& p)
{
    int idx = 0, pn[2] = {1, 1};
    SplitString(p.c_str(), ';', [&](const char* token) {
        if (idx < 2)
            pn[idx++] = atoi(token);
    });
    if (pn[0] <= 0) pn[0] = 1;
    if (pn[1] <= 0) pn[1] = 1;
    if (mModes.DECOM == VT100Modes::DECOM_Absolute) {
        mPx = mFontW*(pn[1]-1);
        mVy = pn[0]-1;
    } else {
        int top = mScrollingRegion.Top;
        int bot = mScrollingRegion.Bottom;
        if (bot < 0) bot = (int)mLines.size()-1;
        if (pn[0]-1 >= top && pn[0]-1 <= bot) {
            mPx = mFontW*(pn[1]-1);
            mVy = pn[0]-1;
        }
    }
}
//
void SerialConnVT100::ProcessHVP(const std::string& p)
{
    ProcessCUP(p);
}
//
void SerialConnVT100::ProcessVT100_IND(const std::string&)
{
    int bot = mScrollingRegion.Bottom;
    if (bot < 0) bot = (int)mLines.size()-1;
    if (mVy == bot) { // scroll up
        int top = mScrollingRegion.Top;
        auto it_end = mLines.begin() + bot + 1;
        Size csz = GetConsoleSize();
        mLines.insert(it_end, VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
        mLines.erase(mLines.begin() + mVx);
    } else {
        mVy++;
    }
}

void SerialConnVT100::ProcessDECSTBM(const std::string& p)
{
    Size csz = GetConsoleSize();
    int idx = 0, pn[2] = {1, 1};
    SplitString(p.c_str(), ';', [=, &idx, &pn](const char* token) {
        if (idx < 2)
            pn[idx++] = atoi(token);
    });
    if (pn[0] <= 0) pn[0] = 1; //
    if (pn[1] <= 0) pn[1] = 1; //
    if (pn[0] < pn[1] && pn[1] <= csz.cy) {
        mScrollingRegion.Top = pn[0]-1;
        mScrollingRegion.Bottom = pn[1]-1;
        // To the home position
        mVx = 0;
        mVy = pn[0]-1;
    }
}
void SerialConnVT100::SaveCursor(CursorData& cd)
{
    cd.Vx = mVx;
    cd.Vy = mVy;
    cd.Px = mPx;
    cd.Py = mPy;
    cd.Charset = mCharset;
    cd.Style = mStyle;
}
void SerialConnVT100::LoadCursor(const CursorData& cd)
{
    mVx = cd.Vx;
    mVy = cd.Vy;
    mPx = cd.Px;
    mPy = cd.Py;
    mCharset = cd.Charset;
    mStyle = cd.Style;
}
void SerialConnVT100::ProcessDECSC(const std::string&)
{
    SaveCursor(mCursorData);
}
void SerialConnVT100::ProcessDECRC(const std::string& p)
{
    LoadCursor(mCursorData);
}

void SerialConnVT100::ProcessDECALN(const std::string&)
{
    for (size_t vy = 0; vy < mLines.size(); ++vy) {
        VTLine& vline = mLines[vy];
        for (size_t vx = 0; vx < vline.size(); ++vx) {
            vline[vx].SetCode('S');
        }
    }
}
void SerialConnVT100::ProcessDECTST(const std::string& p)
{
}
void SerialConnVT100::ProcessDECLL(const std::string& p)
{
}
void SerialConnVT100::ProcessDECREQTPARM(const std::string& p)
{
}
