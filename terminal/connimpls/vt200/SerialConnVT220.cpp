/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT220.h"
#include "VT220ControlSeq.h"
#include "VT220Charset.h"
#include "ConnFactory.h"

REGISTER_CONN_INSTANCE("vt220 by chiv", "vt220", SerialConnVT220);

using namespace Upp;

SerialConnVT220::SerialConnVT220(std::shared_ptr<SerialIo> io)
    : SerialConnVT100(io)
    , SerialConnVT(io)
    , mOperatingLevel(VT200_S7C)
{
    // vt100 supports G0,G1, vt200 supports G2,G3
    mCharsets[2] = CS_DEC_SUPPLEMENTAL;
    mCharsets[3] = CS_DEC_SUPPLEMENTAL;
    this->mExtendedCharset = CS_DEC_SUPPLEMENTAL;
    //
    AddVT220ControlSeqs(this->mSeqsFactory);
    //
    SaveCursorData(mCursorData);
    //
    InstallFunctions();
}

void SerialConnVT220::InstallFunctions()
{
    mFunctions[DECSCL]      = [=](const std::string_view& p) { ProcessDECSCL(p); };
    mFunctions[G2_CS]       = [=](const std::string_view& p) { ProcessG2_CS(p); };
    mFunctions[G3_CS]       = [=](const std::string_view& p) { ProcessG3_CS(p); };
    mFunctions[S7C1T]       = [=](const std::string_view& p) { ProcessS7C1T(p); };
    mFunctions[S8C1T]       = [=](const std::string_view& p) { ProcessS8C1T(p); };
    mFunctions[DECSCA]      = [=](const std::string_view& p) { ProcessDECSCA(p); };
    mFunctions[DECSEL]      = [=](const std::string_view& p) { ProcessDECSEL(p); };
    mFunctions[DECSED]      = [=](const std::string_view& p) { ProcessDECSED(p); };
    mFunctions[DECSTR]      = [=](const std::string_view& p) { ProcessDECSTR(p); };
    mFunctions[SecondaryDA] = [=](const std::string_view& p) { ProcessSecondaryDA(p); };
}
// compatible level
void SerialConnVT220::ProcessDECSCL(const std::string_view& p)
{
    if (p == "61") { // level1, vt100
        mOperatingLevel = VT200_S7C;
    } else if (p == "62" || p == "62;2" || p == "62;0") { // level2, vt200, 8-bit controls
        mOperatingLevel = VT200_S8C;
    } else if (p == "62;1") { // vt200, 7-bit controls
        mOperatingLevel = VT200_S7C;
    }
}
#define DO_SET_CHARSET(g) do { \
    if (p == "<") { \
        mCharsets[g] = CS_DEC_SUPPLEMENTAL; \
    } else if (p == "A") { \
        mCharsets[g] = CS_BRITISH; \
    } else if (p == "4") { \
        mCharsets[g] = CS_DUTCH; \
    } else if (p == "5" || p == "C") { \
        mCharsets[g] = CS_FINNISH; \
    } else if (p == "R") { \
        mCharsets[g] = CS_FRENCH; \
    } else if (p == "Q") { \
        mCharsets[g] = CS_FRENCH_CANADIAN; \
    } else if (p == "K") { \
        mCharsets[g] = CS_GERMAN; \
    } else if (p == "Y") { \
        mCharsets[g] = CS_ITALIAN; \
    } else if (p == "E" || p == "6") { \
        mCharsets[g] = CS_DANISH; \
    } else if (p == "Z") { \
        mCharsets[g] = CS_SPANISH; \
    } else if (p == "H" || p == "7") { \
        mCharsets[g] = CS_SWEDISH; \
    } else if (p == "=") { \
        mCharsets[g] = CS_SWISS; \
    } else if (p == "B") { \
        mCharsets[g] = CS_ASCII; \
    } else if (p == "0") { \
        mCharsets[g] = CS_DEC_SPECIAL_GRAPHICS; \
    } else { \
        mCharsets[g] = CS_DEFAULT; \
    } \
    mCharset = mCharsets[g]; \
} while (0)
void SerialConnVT220::ProcessG0_CS(const std::string_view& p)
{
    DO_SET_CHARSET(0);
}
void SerialConnVT220::ProcessG1_CS(const std::string_view& p)
{
    DO_SET_CHARSET(1);
}
void SerialConnVT220::ProcessG2_CS(const std::string_view& p)
{
    DO_SET_CHARSET(2);
}
void SerialConnVT220::ProcessG3_CS(const std::string_view& p)
{
    DO_SET_CHARSET(3);
}
// S8C will break the UTF-8 sequences, so we do not support S8C.
void SerialConnVT220::ProcessS7C1T(const std::string_view&)
{
    SetUseS8C(false);
}
void SerialConnVT220::ProcessS8C1T(const std::string_view&)
{
    SetUseS8C(true);
}

void SerialConnVT220::ProcessDECSC(const std::string_view&)
{
    SaveCursorData(mCursorData);
}
void SerialConnVT220::ProcessDECRC(const std::string_view&)
{
    LoadCursorData(mCursorData);
}

void SerialConnVT220::ProcessDECSCA(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 0: this->SetDefaultStyle(); mCursorData.SelectiveErase = 1; break;
    case 1: mCursorData.SelectiveErase = 0; break;
    case 2: mCursorData.SelectiveErase = 1; break;
    default: break;
    }
}
void SerialConnVT220::ProcessDECSEL(const std::string_view&)
{
    if (mCursorData.SelectiveErase) {
        VTLine& vline = mLines[mVy];
        for (int i = mVx; i < (int)vline.size(); ++i) {
            vline[i] = ' ';
        }
    }
}
void SerialConnVT220::ProcessDECSED(const std::string_view&)
{
    if (mCursorData.SelectiveErase) {
        // erase char, do not erase the style
        VTLine& vline = mLines[mVy];
        for (int i = mVx; i < (int)vline.size(); ++i) {
            vline[i] = ' ';
        }
        for (int y = mVy+1; y < (int)mLines.size(); ++y) {
            for (int x = 0; x < (int)mLines[y].size(); ++x) {
                mLines[y][x] = ' ';
            }
        }
    }
}
void SerialConnVT220::ProcessDECSTR(const std::string_view&)
{
    // default state
    SetShowCursor(true);
    SerialConnVT100::mModes.DECAWM = VT100Modes::OFF;
    SetWrapLine(false);
    this->mScrollingRegion.Top = 0;
    this->mScrollingRegion.Bottom = 23;
    //
    Clear();
}
void SerialConnVT220::ProcessSecondaryDA(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 0:
        Put("\x1b[>1;10;0c"); // VT220 version 1.0, no options
        break;
    }
}
//
void SerialConnVT220::SaveCursorData(CursorDataVT220& cd)
{
    SerialConnVT100::SaveCursorData(cd);
    cd.SelectiveErase = mCursorData.SelectiveErase;
}
void SerialConnVT220::LoadCursorData(const CursorDataVT220& cd)
{
    SerialConnVT100::LoadCursorData(cd);
    mCursorData.SelectiveErase = cd.SelectiveErase;
}

void SerialConnVT220::ProcessDECDSR(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 26:
        Put("\E[?27;1n"); // set keyboard language to "North American"
        break;
    default:
        SerialConnVT100::ProcessDECDSR(p);
        break;
    }
}

void SerialConnVT220::ProcessDECSM(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 25: SetShowCursor(true); break;
    case 42: mModes.DECNRCM = 1; break;
    default: SerialConnVT100::ProcessDECSM(p); break;
    }
}
void SerialConnVT220::ProcessDECRM(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 25: SetShowCursor(false); break;
    case 42: mModes.DECNRCM = 0; break;
    default: SerialConnVT100::ProcessDECRM(p); break;
    }
}
void SerialConnVT220::ProcessSS2(const std::string_view&)
{
    mCharset = mCharsets[2];
}
void SerialConnVT220::ProcessSS3(const std::string_view&)
{
    mCharset = mCharsets[3];
}
void SerialConnVT220::ProcessLS2(const std::string_view&)
{
    mCharset = mCharsets[2];
}
void SerialConnVT220::ProcessLS3(const std::string_view&)
{
    mCharset = mCharsets[3];
}
// for 0x80-0xff, we call them extended cs, we do not use them actually,
// because we treat all of inputs as UTF-8 default. Programs who did not
// support UTF-8 were too old to support well.
void SerialConnVT220::ProcessLS1R(const std::string_view&)
{
    mExtendedCharset = mCharsets[1];
}
void SerialConnVT220::ProcessLS2R(const std::string_view&)
{
    mExtendedCharset = mCharsets[2];
}
void SerialConnVT220::ProcessLS3R(const std::string_view&)
{
    mExtendedCharset = mCharsets[3];
}
//
uint32_t SerialConnVT220::RemapCharacter(uint32_t uc, int charset)
{
    return VT220_RemapCharacter(uc, charset, mExtendedCharset);
}

bool SerialConnVT220::ProcessKeyDown(Upp::dword key, Upp::dword flags)
{
    bool processed = false;
    if (flags == 0 && mOperatingLevel > VT100) {
        processed = true;
        switch (key) {
        case K_SPACE:
            if (SerialConnVT100::mModes.DECKPM == VT100Modes::DECKPM_PNM) {
                Put(" ");
            } else {
                Put("\EO ");
            }
            break;
        case K_TAB:
            if (SerialConnVT100::mModes.DECKPM == VT100Modes::DECKPM_PNM) {
                Put("\t");
            } else {
                Put("\EOI");
            }
            break;
        case K_MULTIPLY:
            if (SerialConnVT100::mModes.DECKPM == VT100Modes::DECKPM_PNM) {
                Put("*");
            } else {
                Put("\EOj");
            }
            break;
        case K_ADD:
            if (SerialConnVT100::mModes.DECKPM == VT100Modes::DECKPM_PNM) {
                Put("+");
            } else {
                Put("\EOk");
            }
            break;
        case K_DIVIDE:
            if (SerialConnVT100::mModes.DECKPM == VT100Modes::DECKPM_PNM) {
                Put("/");
            } else {
                Put("\033Oo");
            }
            break;
        case '=':
            if (SerialConnVT100::mModes.DECKPM == VT100Modes::DECKPM_PNM) {
                Put("=");
            } else {
                Put("\033OX");
            }
            break;
        case K_F6: Put("\E[17~"); break;
        case K_F7: Put("\E[18~"); break;
        case K_F8: Put("\E[19~"); break;
        case K_F9: Put("\E[20~"); break;
        case K_F10: Put("\E[21~"); break;
        case K_F11: Put("\E[23~"); break;
        case K_F12: Put("\E[24~"); break;
        // vt220 provides 6 key editing keypad.
        case K_HOME: Put("\E[1~"); break;
        case K_END: Put("\E[4~"); break;
        case K_DELETE: Put("\E[3~"); break; // vt220, Remove
        case K_INSERT: Put("\E[2~"); break; // vt220, Insert
        case K_PAGEUP: Put("\E[5~"); break; // vt220, prev screen
        case K_PAGEDOWN: Put("\E[6~"); break; // vt220, next screen
        default:
            processed = false;
            break;
        }
    }
    return processed ? true : SerialConnVT100::ProcessKeyDown(key, flags);
}
