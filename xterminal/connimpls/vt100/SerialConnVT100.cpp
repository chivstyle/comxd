/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT100.h"
#include "ConnFactory.h"
#include "VT100Charset.h"
#include "VT100ControlSeq.h"

using namespace Upp;
using namespace xvt;

REGISTER_CONN_INSTANCE("vt100", "vt100", SerialConnVT100);

SerialConnVT100::SerialConnVT100(std::shared_ptr<SerialIo> io)
    : SerialConnVT(io)
    , SerialConnEcma48(io)
    , mKeypadMode(KM_Normal)
{
    SetConnDescription("vt100 emulator, by chiv, v1.0a");
    //
    AddVT100ControlSeqs(this->mSeqsFactory);
    // VT100, permanently selected modes
    LoadDefaultModes();
    // enable wrap line
    SetWrapLine(true);
    // default charsets
    mCharsets[0] = CS_US;
    mCharsets[1] = CS_UK;
    //
    InstallFunctions();
}

void SerialConnVT100::LoadDefaultModes()
{
    mModes.SetDecpMode(DECCKM, 0); // cursor
    mModes.SetDecpMode(DECANM, 1); // ANSI
    mModes.SetDecpMode(DECOM,  0); // Absolute
    mModes.SetDecpMode(DECAWM, 1); // Auto wrap line
    // others modes were ignored, we do not support them.
}

void SerialConnVT100::InstallFunctions()
{
    mFunctions[DECKPNM] = [=](const std::string& p) { ProcessDECKPNM(p); };
    mFunctions[DECKPAM] = [=](const std::string& p) { ProcessDECKPAM(p); };
    //
    mFunctions[DECSM] = [=](const std::string& p) { ProcessDECSM(p); };
    mFunctions[DECRM] = [=](const std::string& p) { ProcessDECRM(p); };
    mFunctions[DECDSR] = [=](const std::string& p) { ProcessDECDSR(p); };
    //
    mFunctions[G0_CS] = [=](const std::string& p) { ProcessG0_CS(p); };
    mFunctions[G1_CS] = [=](const std::string& p) { ProcessG1_CS(p); };
    //
    mFunctions[DECREQTPARM] = [=](const std::string& p) { ProcessDECREQTPARM(p); };
    mFunctions[DECSTBM] = [=](const std::string& p) { ProcessDECSTBM(p); };
    mFunctions[DECSC] = [=](const std::string& p) {
        ProcessDECSC(p);
    };
    mFunctions[DECRC] = [=](const std::string& p) { ProcessDECRC(p); };
    mFunctions[DECALN] = [=](const std::string& p) { ProcessDECALN(p); };
    mFunctions[DECTST] = [=](const std::string& p) { ProcessDECTST(p); };
    mFunctions[DECLL] = [=](const std::string& p) { ProcessDECLL(p); };
    mFunctions[DECIND] = [=](const std::string& p) { ProcessDECIND(p); };
}
//
void SerialConnVT100::ProcessDECKPNM(const std::string&)
{
    mKeypadMode = KM_Normal;
}
void SerialConnVT100::ProcessDECKPAM(const std::string&)
{
    mKeypadMode = KM_Application;
}
//
void SerialConnVT100::ProcessDA(const std::string& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 0:
        // https://www.vt100.net/docs/vt100-ug/chapter3.html#DA
        Put("\033[?1;2c"); // Advanced video option
        break;
    }
}
//
#define DO_SET_CHARSET(g)                   \
    do {                                    \
        if (p == "A") {                     \
            mCharsets[g] = CS_UK;           \
        } else if (p == "B") {              \
            mCharsets[g] = CS_US;           \
        } else if (p == "0") {              \
            mCharsets[g] = CS_LINE_DRAWING; \
        } else if (p == "1") {              \
            mCharsets[g] = CS_ROM;          \
        } else if (p == "2") {              \
            mCharsets[g] = CS_ROM_SPECIAL;  \
        } else {                            \
            mCharsets[g] = CS_DEFAULT;      \
        }                                   \
        mCharset = mCharsets[g];            \
    } while (0)
//
void SerialConnVT100::ProcessG0_CS(const std::string& p)
{
    DO_SET_CHARSET(0);
}
void SerialConnVT100::ProcessG1_CS(const std::string& p)
{
    DO_SET_CHARSET(1);
}
void SerialConnVT100::ProcessSI(const std::string&)
{
    mCharset = mCharsets[0];
}
void SerialConnVT100::ProcessSO(const std::string&)
{
    mCharset = mCharsets[1];
}
uint32_t SerialConnVT100::RemapCharacter(uint32_t uc, int charset)
{
    if (uc >= ' ' && uc < 0x7f) {
        return VT100_RemapCharacter(uc, charset);
    }
    return SerialConnEcma48::RemapCharacter(uc, charset);
}
//
bool SerialConnVT100::ProcessKeyDown(Upp::dword key, Upp::dword flags)
{
    bool processed = false;
    if (flags == 0) {
        processed = true;
        switch (key) {
        case K_UP:
            if (mModes.GetDecpMode(DECCKM) == 0) {
                Put("\033[A");
            } else {
                Put("\033OA");
            }
            break;
        case K_DOWN:
            if (mModes.GetDecpMode(DECCKM) == 0) {
                Put("\033[B");
            } else {
                Put("\033OB");
            }
            break;
        case K_LEFT:
            if (mModes.GetDecpMode(DECCKM) == 0) {
                Put("\033[D");
            } else {
                Put("\033OD");
            }
            break;
        case K_RIGHT:
            if (mModes.GetDecpMode(DECCKM) == 0) {
                Put("\033[C");
            } else {
                Put("\033OC");
            }
            break;
        /*! PF1 ~ PF4 */
        case K_F1:
            if (mModes.GetDecpMode(DECANM) == 1) {
                Put("\033OP");
            } else {
                Put("\033P");
            }
            break;
        case K_F2:
            if (mModes.GetDecpMode(DECANM) == 1) {
                Put("\033OQ");
            } else {
                Put("\033Q");
            }
            break;
        case K_F3:
            if (mModes.GetDecpMode(DECANM) == 1) {
                Put("\033OR");
            } else {
                Put("\033R");
            }
            break;
        case K_F4:
            if (mModes.GetDecpMode(DECANM) == 1) {
                Put("\033OS");
            } else {
                Put("\033S");
            }
            break;
        /*! keys below, I'm not sure, from libncurse */
        case K_F5:
            Put("\033Ot");
            break;
        case K_F6:
            Put("\033Ou");
            break;
        case K_F7:
            Put("\033Ov");
            break;
        case K_F8:
            Put("\033Ol");
            break;
        case K_F9:
            Put("\033Ow");
            break;
        case K_F10:
            Put("\033Ox");
            break;
        case K_HOME:
            Put("\033[H");
            break;
        default:
            processed = false;
            break;
        }
    }
    return processed ? true : SerialConnEcma48::ProcessKeyDown(key, flags);
}
//
void SerialConnVT100::ProcessDECSM(const std::string& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 5:
        if (mModes.GetDecpMode(DECSCNM, 0) == 0) {
            mColorTbl.Swap(VTColorTable::kColorId_Paper, VTColorTable::kColorId_Texts);
        }
        break;
    case 6:
        SetCursorToHome();
        break;
    case 7:
        SetWrapLine(true);
        break;
    }
    mModes.SetDecpMode(ps, 1);
}
void SerialConnVT100::ProcessDECRM(const std::string& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 5:
        if (mModes.GetDecpMode(DECSCNM) == 1) {
            mColorTbl.Swap(VTColorTable::kColorId_Paper, VTColorTable::kColorId_Texts);
        }
        break;
    case 6:
        SetCursorToHome();
        break;
    case 7:
        SetWrapLine(false);
        break;
    }
    mModes.SetDecpMode(ps, 0);
}
//
void SerialConnVT100::ProcessDECDSR(const std::string& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 15:
        Put("\033[?13n"); // No printer
        break;
    }
}
void SerialConnVT100::SetCursorToHome()
{
    if (mModes.GetDecpMode(DECOM) == 1) { // Set - Absolute
         mVx = 0; mVy = 0;
    } else {
        int top = mScrollingRegion.Top;
        int bot = mScrollingRegion.Bottom;
        if (bot < 0)
            bot = (int)mLines.size() - 1;
        mVx = 0;
        mVy = top;
    }
}
//
void SerialConnVT100::ProcessCUP(const std::string& p)
{
    int idx = 0, pn[2] = { 1, 1 };
    SplitString(p.data(), ";", [&](const char* token) {
        if (idx < 2)
            pn[idx++] = atoi(token);
    });
    if (pn[0] <= 0)
        pn[0] = 1;
    if (pn[1] <= 0)
        pn[1] = 1;
    if (mModes.GetDecpMode(DECOM) == 1) {
        mPx = mFontW * (pn[1] - 1);
        mVy = pn[0] - 1;
    } else {
        int top = mScrollingRegion.Top;
        int bot = mScrollingRegion.Bottom;
        if (bot < 0)
            bot = (int)mLines.size() - 1;
        // check and fix
        Size csz = GetConsoleSize();
        if (pn[0] - 1 < top) pn[0] = top + 1;
        else if (pn[0] - 1 > bot) pn[0] = bot + 1;
        if (pn[1] - 1 < 0) pn[1] = 1;
        else if (pn[1] > csz.cx) pn[1] = csz.cx;
        
        mPx = mFontW * (pn[1] - 1);
        mVy = pn[0] - 1;
    }
}
//
void SerialConnVT100::ProcessHVP(const std::string& p)
{
    ProcessCUP(p);
}
//
void SerialConnVT100::ProcessDECIND(const std::string&)
{
    int bot = mScrollingRegion.Bottom;
    if (bot < 0)
        bot = (int)mLines.size() - 1;
    if (mVy < bot) {
        mVy++;
    } else { // scroll up
        int top = mScrollingRegion.Top;
        if (bot < 0)
            bot = (int)mLines.size() - 1;
        Size csz = GetConsoleSize();
        auto it_end = mLines.begin() + bot + 1;
        // insert new line
        mLines.insert(it_end, VTLine(csz.cx, GetBlankChar()).SetHeight(mFontH));
        // remove the top line
        mLines.erase(mLines.begin() + top);
    }
}
//
void SerialConnVT100::ProcessDECSTBM(const std::string& p)
{
    Size csz = GetConsoleSize();
    int idx = 0, pn[2] = { 1, 1 };
    SplitString(p.data(), ";", [=, &idx, &pn](const char* token) {
        if (idx < 2)
            pn[idx++] = atoi(token);
    });
    if (pn[0] <= 0)
        pn[0] = 1; //
    if (pn[1] <= 0)
        pn[1] = 1; //
    if (pn[0] < pn[1] && pn[1] <= csz.cy) {
        mScrollingRegion.Top = pn[0] - 1;
        mScrollingRegion.Bottom = pn[1] - 1;
        // To the home position
        mVx = 0;
        mVy = pn[0] - 1;
    }
}
void SerialConnVT100::SaveCursorData(CursorDataVT100& cd)
{
    cd.Vx = mVx;
    cd.Vy = mVy;
    cd.Px = mPx;
    cd.Py = mPy;
    cd.Charset = mCharset;
    cd.Style = mStyle;
}
void SerialConnVT100::LoadCursorData(const CursorDataVT100& cd)
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
    SaveCursorData(mCursorData);
}
void SerialConnVT100::ProcessDECRC(const std::string&)
{
    LoadCursorData(mCursorData);
}
//
void SerialConnVT100::ProcessDECALN(const std::string&)
{
    SetCursorToHome();
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
