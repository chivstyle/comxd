/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT220.h"
#include "ConnFactory.h"
#include "VT220Charset.h"
#include "VT220ControlSeq.h"

using namespace Upp;
using namespace xvt;
//REGISTER_CONN_INSTANCE("vt220 by chiv", "vt220", SerialConnVT220);

SerialConnVT220::SerialConnVT220(std::shared_ptr<SerialIo> io)
    : SerialConnVT100(io)
    , SerialConnVT(io)
{
    SetConnDescription("vt220 emulator, by chiv, v1.0a");
    //
    SetOperatingLevel(VT200_S7C);
    // vt100 supports G0,G1, vt200 supports G2,G3
    mCharsets[2] = CS_DEC_SUPPLEMENTAL;
    mCharsets[3] = CS_DEC_SUPPLEMENTAL;
    this->mExtendedCharset = CS_DEC_SUPPLEMENTAL;
    //
    AddVT220ControlSeqs(this->mSeqsFactory);
    //
    LoadDefaultModes();
    //
    SaveCursorData(mCursorData);
    //
    InstallFunctions();
}

void SerialConnVT220::LoadDefaultModes()
{
	mModes.SetDecpMode(DECTCEM, 1);
}

void SerialConnVT220::InstallFunctions()
{
    mFunctions[DECSCL] = [=](const std::string& p) { ProcessDECSCL(p); };
    mFunctions[G2_CS] = [=](const std::string& p) { ProcessG2_CS(p); };
    mFunctions[G3_CS] = [=](const std::string& p) { ProcessG3_CS(p); };
    mFunctions[S7C1T] = [=](const std::string& p) { ProcessS7C1T(p); };
    mFunctions[S8C1T] = [=](const std::string& p) { ProcessS8C1T(p); };
    mFunctions[DECSCA] = [=](const std::string& p) { ProcessDECSCA(p); };
    mFunctions[DECSEL] = [=](const std::string& p) { ProcessDECSEL(p); };
    mFunctions[DECSED] = [=](const std::string& p) { ProcessDECSED(p); };
    mFunctions[DECSTR] = [=](const std::string& p) { ProcessDECSTR(p); };
    mFunctions[SecondaryDA] = [=](const std::string& p) { ProcessSecondaryDA(p); };
}
// compatible level
void SerialConnVT220::ProcessDECSCL(const std::string& p)
{
    int idx = 0;
    int ps[2] = {0, 0};
    SplitString(p.data(), ";", [=, &idx, &ps](const char* token) {
        if (idx < 2)
            ps[idx] = atoi(token);
        idx++;
    });
    int level = 0;
    if (ps[0] <= 62) { // map to VT200
        level = VT200_S7C;
        if (ps[1] == 0 || ps[1] == 2)
            level |= VTFLG_S8C;
        //
        SetOperatingLevel(level);
    }
}
#define DO_SET_CHARSET(g)                           \
    do {                                            \
        if (p == "<") {                             \
            mCharsets[g] = CS_DEC_SUPPLEMENTAL;     \
        } else if (p == "A") {                      \
            mCharsets[g] = CS_BRITISH;              \
        } else if (p == "4") {                      \
            mCharsets[g] = CS_DUTCH;                \
        } else if (p == "5" || p == "C") {          \
            mCharsets[g] = CS_FINNISH;              \
        } else if (p == "R") {                      \
            mCharsets[g] = CS_FRENCH;               \
        } else if (p == "Q") {                      \
            mCharsets[g] = CS_FRENCH_CANADIAN;      \
        } else if (p == "K") {                      \
            mCharsets[g] = CS_GERMAN;               \
        } else if (p == "Y") {                      \
            mCharsets[g] = CS_ITALIAN;              \
        } else if (p == "E" || p == "6") {          \
            mCharsets[g] = CS_DANISH;               \
        } else if (p == "Z") {                      \
            mCharsets[g] = CS_SPANISH;              \
        } else if (p == "H" || p == "7") {          \
            mCharsets[g] = CS_SWEDISH;              \
        } else if (p == "=") {                      \
            mCharsets[g] = CS_SWISS;                \
        } else if (p == "B") {                      \
            mCharsets[g] = CS_ASCII;                \
        } else if (p == "0") {                      \
            mCharsets[g] = CS_DEC_SPECIAL_GRAPHICS; \
        } else {                                    \
            mCharsets[g] = CS_DEFAULT;              \
        }                                           \
        mCharset = mCharsets[g];                    \
    } while (0)
void SerialConnVT220::ProcessG0_CS(const std::string& p)
{
    DO_SET_CHARSET(0);
}
void SerialConnVT220::ProcessG1_CS(const std::string& p)
{
    DO_SET_CHARSET(1);
}
void SerialConnVT220::ProcessG2_CS(const std::string& p)
{
    DO_SET_CHARSET(2);
}
void SerialConnVT220::ProcessG3_CS(const std::string& p)
{
    DO_SET_CHARSET(3);
}
// S8C will break the UTF-8 sequences, so we do not support S8C.
void SerialConnVT220::ProcessS7C1T(const std::string&)
{
    SetUseS8C(false);
}
void SerialConnVT220::ProcessS8C1T(const std::string&)
{
    SetUseS8C(true);
}

void SerialConnVT220::ProcessDECSC(const std::string&)
{
    SaveCursorData(mCursorData);
}
void SerialConnVT220::ProcessDECRC(const std::string&)
{
    LoadCursorData(mCursorData);
}

void SerialConnVT220::ProcessDECSCA(const std::string& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 0:
        this->SetDefaultStyle();
        mCursorData.SelectiveErase = 1;
        break;
    case 1:
        mCursorData.SelectiveErase = 0;
        break;
    case 2:
        mCursorData.SelectiveErase = 1;
        break;
    default:
        break;
    }
}
void SerialConnVT220::ProcessDECSEL(const std::string&)
{
    if (mCursorData.SelectiveErase) {
        VTLine& vline = mLines[mVy];
        for (int i = mVx; i < (int)vline.size(); ++i) {
            vline[i] = ' ';
        }
    }
}
void SerialConnVT220::ProcessDECSED(const std::string&)
{
    if (mCursorData.SelectiveErase) {
        // erase char, do not erase the style
        VTLine& vline = mLines[mVy];
        for (int i = mVx; i < (int)vline.size(); ++i) {
            vline[i] = ' ';
        }
        for (int y = mVy + 1; y < (int)mLines.size(); ++y) {
            for (int x = 0; x < (int)mLines[y].size(); ++x) {
                mLines[y][x] = ' ';
            }
        }
    }
}
void SerialConnVT220::ProcessDECSTR(const std::string&)
{
    // default state
    SetShowCursor(true);
    mModes.SetDecpMode(DECAWM, 0);
    SetWrapLine(false);
    this->mScrollingRegion.Top = 0;
    this->mScrollingRegion.Bottom = 23;
    //
    ClearVt();
}
void SerialConnVT220::ProcessSecondaryDA(const std::string& p)
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

void SerialConnVT220::ProcessDECDSR(const std::string& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 26:
        Put("\033[?27;1n"); // set keyboard language to "North American"
        break;
    default:
        SerialConnVT100::ProcessDECDSR(p);
        break;
    }
}

void SerialConnVT220::ProcessDECSM(const std::string& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 25:
        SetShowCursor(true);
        mModes.SetDecpMode(ps, 1);
        break;
    default:
        SerialConnVT100::ProcessDECSM(p);
        break;
    }
}
void SerialConnVT220::ProcessDECRM(const std::string& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 25:
        SetShowCursor(false);
        mModes.SetDecpMode(ps, 0);
        break;
    default:
        SerialConnVT100::ProcessDECRM(p);
        break;
    }
}
void SerialConnVT220::ProcessSS2(const std::string&)
{
    mCharset = mCharsets[2];
}
void SerialConnVT220::ProcessSS3(const std::string&)
{
    mCharset = mCharsets[3];
}
void SerialConnVT220::ProcessLS2(const std::string&)
{
    mCharset = mCharsets[2];
}
void SerialConnVT220::ProcessLS3(const std::string&)
{
    mCharset = mCharsets[3];
}
// for 0x80-0xff, we call them extended cs, we do not use them actually,
// because we treat all of inputs as UTF-8 default. Programs who did not
// support UTF-8 were too old to support well.
void SerialConnVT220::ProcessLS1R(const std::string&)
{
    mExtendedCharset = mCharsets[1];
}
void SerialConnVT220::ProcessLS2R(const std::string&)
{
    mExtendedCharset = mCharsets[2];
}
void SerialConnVT220::ProcessLS3R(const std::string&)
{
    mExtendedCharset = mCharsets[3];
}
//
uint32_t SerialConnVT220::RemapCharacter(uint32_t uc, int charset)
{
    return VT220_RemapCharacter(uc, charset, mExtendedCharset);
}
//
void SerialConnVT220::SetOperatingLevel(int level)
{
    mOperatingLevel = level;
    SetUseS8C((level & (0x1 << 16)) > 0);
}
int SerialConnVT220::GetOperatingLevel() const
{
    return mOperatingLevel;
}

bool SerialConnVT220::ProcessKeyDown(Upp::dword key, Upp::dword flags)
{
    bool processed = false;
    if (flags == 0 && GetOperatingLevel() > VT100_S7C) {
        processed = true;
        switch (key) {
        case K_F6:
            Put("\033[17~");
            break;
        case K_F7:
            Put("\033[18~");
            break;
        case K_F8:
            Put("\033[19~");
            break;
        case K_F9:
            Put("\033[20~");
            break;
        case K_F10:
            Put("\033[21~");
            break;
        case K_F11:
            Put("\033[23~");
            break;
        case K_F12:
            Put("\033[24~");
            break;
        // vt220 provides 6 key editing keypad.
        case K_HOME:
            Put("\033[1~");
            break;
        case K_END:
            Put("\033[4~");
            break;
        case K_DELETE:
            Put("\033[3~");
            break; // vt220, Remove
        case K_INSERT:
            Put("\033[2~");
            break; // vt220, Insert
        case K_PAGEUP:
            Put("\033[5~");
            break; // vt220, prev screen
        case K_PAGEDOWN:
            Put("\033[6~");
            break; // vt220, next screen
        default:
            processed = false;
            break;
        }
    }
    return processed ? true : SerialConnVT100::ProcessKeyDown(key, flags);
}
