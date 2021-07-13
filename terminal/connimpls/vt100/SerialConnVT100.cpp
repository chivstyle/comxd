/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT100.h"
#include "ConnFactory.h"
#include "VT100Charset.h"
#include "VT100ControlSeq.h"

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
    SetWrapLine(true);
    // default charsets
    mCharsets[0] = CS_US;
    mCharsets[1] = CS_UK;
    //
    InstallFunctions();
}

void SerialConnVT100::InstallFunctions()
{
    mFunctions[DECKPNM] = [=](const std::string_view& p) { ProcessDECKPNM(p); };
    mFunctions[DECKPAM] = [=](const std::string_view& p) { ProcessDECKPAM(p); };
    //
    mFunctions[DECSM] = [=](const std::string_view& p) { ProcessDECSM(p); };
    mFunctions[DECRM] = [=](const std::string_view& p) { ProcessDECRM(p); };
    mFunctions[DECDSR] = [=](const std::string_view& p) { ProcessDECDSR(p); };
    //
    mFunctions[G0_CS] = [=](const std::string_view& p) { ProcessG0_CS(p); };
    mFunctions[G1_CS] = [=](const std::string_view& p) { ProcessG1_CS(p); };
    //
    mFunctions[DECREQTPARM] = [=](const std::string_view& p) { ProcessDECREQTPARM(p); };
    mFunctions[DECSTBM] = [=](const std::string_view& p) { ProcessDECSTBM(p); };
    mFunctions[DECSC] = [=](const std::string_view& p) { ProcessDECSC(p); };
    mFunctions[DECRC] = [=](const std::string_view& p) { ProcessDECRC(p); };
    mFunctions[DECALN] = [=](const std::string_view& p) { ProcessDECALN(p); };
    mFunctions[DECTST] = [=](const std::string_view& p) { ProcessDECTST(p); };
    mFunctions[DECLL] = [=](const std::string_view& p) { ProcessDECLL(p); };
    mFunctions[DECIND] = [=](const std::string_view& p) { ProcessDECIND(p); };
}
//
void SerialConnVT100::ProcessDECKPNM(const std::string_view&)
{
    mModes.DECKPM = VT100Modes::DECKPM_PNM;
}
void SerialConnVT100::ProcessDECKPAM(const std::string_view&)
{
    mModes.DECKPM = VT100Modes::DECKPM_PAM;
}
//
void SerialConnVT100::ProcessDA(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 0:
        // https://www.vt100.net/docs/vt100-ug/chapter3.html#DA
        Put("\E[?1;2c"); // Advanced video option
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
void SerialConnVT100::ProcessG0_CS(const std::string_view& p)
{
    DO_SET_CHARSET(0);
}
void SerialConnVT100::ProcessG1_CS(const std::string_view& p)
{
    DO_SET_CHARSET(1);
}
void SerialConnVT100::ProcessSI(const std::string_view&)
{
    mCharset = mCharsets[0];
}
void SerialConnVT100::ProcessSO(const std::string_view&)
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
            if (mModes.DECCKM == VT100Modes::DECCKM_Cursor) {
                Put("\E[A");
            } else {
                Put("\EOA");
            }
            break;
        case K_DOWN:
            if (mModes.DECCKM == VT100Modes::DECCKM_Cursor) {
                Put("\E[B");
            } else {
                Put("\EOB");
            }
            break;
        case K_LEFT:
            if (mModes.DECCKM == VT100Modes::DECCKM_Cursor) {
                Put("\E[D");
            } else {
                Put("\EOD");
            }
            break;
        case K_RIGHT:
            if (mModes.DECCKM == VT100Modes::DECCKM_Cursor) {
                Put("\E[C");
            } else {
                Put("\EOC");
            }
            break;
        /*! PF1 ~ PF4 */
        case K_F1:
            if (mModes.DECANM == VT100Modes::DECANM_ANSI) {
                Put("\EOP");
            } else {
                Put("\EP");
            }
            break;
        case K_F2:
            if (mModes.DECANM == VT100Modes::DECANM_ANSI) {
                Put("\EOQ");
            } else {
                Put("\EQ");
            }
            break;
        case K_F3:
            if (mModes.DECANM == VT100Modes::DECANM_ANSI) {
                Put("\EOR");
            } else {
                Put("\ER");
            }
            break;
        case K_F4:
            if (mModes.DECANM == VT100Modes::DECANM_ANSI) {
                Put("\EOS");
            } else {
                Put("\ES");
            }
            break;
        /*! keys below, I'm not sure, from libncurse */
        case K_F5:
            Put("\EOt");
            break;
        case K_F6:
            Put("\EOu");
            break;
        case K_F7:
            Put("\EOv");
            break;
        case K_F8:
            Put("\EOl");
            break;
        case K_F9:
            Put("\EOw");
            break;
        case K_F10:
            Put("\EOx");
            break;
        case K_HOME:
            Put("\E[H");
            break;
        default:
            processed = false;
            break;
        }
    }
    return processed ? true : SerialConnEcma48::ProcessKeyDown(key, flags);
}
//
void SerialConnVT100::ProcessDECSM(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 1:
        mModes.DECCKM = 1;
        break;
    case 3:
        mModes.DECCOLM = 1;
        break;
    case 4:
        mModes.DECSCLM = 1;
        break;
    case 5:
        if (!mModes.DECSCNM) {
            mModes.DECSCNM = 1;
            mColorTbl.Swap(VTColorTable::kColorId_Paper, VTColorTable::kColorId_Texts);
        }
        break;
    case 6:
        mModes.DECOM = 1;
        SetCursorToHome();
        break;
    case 7:
        mModes.DECAWM = 1;
        SetWrapLine(true);
        break;
    case 8:
        mModes.DECARM = 1;
        break;
    case 18:
        mModes.DECPFF = 1;
        break;
    case 19:
        mModes.DECPEX = 1;
        break;
    }
}
void SerialConnVT100::ProcessDECRM(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 1:
        mModes.DECCKM = 0;
        break;
    case 2:
        mModes.DECANM = 0;
        break;
    case 3:
        mModes.DECCOLM = 0;
        break;
    case 4:
        mModes.DECSCLM = 0;
        break;
    case 5:
        if (mModes.DECSCNM) {
            mModes.DECSCNM = 0;
            mColorTbl.Swap(VTColorTable::kColorId_Paper, VTColorTable::kColorId_Texts);
        }
        break;
    case 6:
        mModes.DECOM = 0;
        SetCursorToHome();
        break;
    case 7:
        mModes.DECAWM = 0;
        SetWrapLine(false);
        break;
    case 8:
        mModes.DECARM = 0;
        break;
    case 18:
        mModes.DECPFF = 0;
        break;
    case 19:
        mModes.DECPEX = 0;
        break;
    }
}
//
void SerialConnVT100::ProcessDECDSR(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 15:
        Put("\E[?13n"); // No printer
        break;
    }
}
void SerialConnVT100::SetCursorToHome()
{
    if (mModes.DECOM == VT100Modes::DECOM_Absolute) {
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
void SerialConnVT100::ProcessCUP(const std::string_view& p)
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
    if (mModes.DECOM == VT100Modes::DECOM_Absolute) {
        mPx = mFontW * (pn[1] - 1);
        mVy = pn[0] - 1;
    } else {
        int top = mScrollingRegion.Top;
        int bot = mScrollingRegion.Bottom;
        if (bot < 0)
            bot = (int)mLines.size() - 1;
        if (pn[0] - 1 >= top && pn[0] - 1 <= bot) {
            mPx = mFontW * (pn[1] - 1);
            mVy = pn[0] - 1;
        }
    }
}
//
void SerialConnVT100::ProcessHVP(const std::string_view& p)
{
    ProcessCUP(p);
}
//
void SerialConnVT100::ProcessDECIND(const std::string_view&)
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
        mLines.insert(it_end, VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
        // remove the top line
        mLines.erase(mLines.begin() + top);
    }
}
//
void SerialConnVT100::ProcessDECSTBM(const std::string_view& p)
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
void SerialConnVT100::ProcessDECSC(const std::string_view&)
{
    SaveCursorData(mCursorData);
}
void SerialConnVT100::ProcessDECRC(const std::string_view&)
{
    LoadCursorData(mCursorData);
}
//
void SerialConnVT100::ProcessDECALN(const std::string_view&)
{
    SetCursorToHome();
    for (size_t vy = 0; vy < mLines.size(); ++vy) {
        VTLine& vline = mLines[vy];
        for (size_t vx = 0; vx < vline.size(); ++vx) {
            vline[vx].SetCode('S');
        }
    }
}
void SerialConnVT100::ProcessDECTST(const std::string_view& p)
{
}
void SerialConnVT100::ProcessDECLL(const std::string_view& p)
{
}
void SerialConnVT100::ProcessDECREQTPARM(const std::string_view& p)
{
}
