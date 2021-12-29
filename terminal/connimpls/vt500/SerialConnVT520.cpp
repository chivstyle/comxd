/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT520.h"
#include "ConnFactory.h"
#include "VT520Charset.h"
#include "VT520ControlSeq.h"

//REGISTER_CONN_INSTANCE("vt520 by chiv", "vt520", SerialConnVT520);

using namespace Upp;

SerialConnVT520::SerialConnVT520(std::shared_ptr<SerialIo> io)
    : SerialConnVT420(io)
    , SerialConnVT(io)
{
    SetConnDescription("vt520 emulator, by chiv, v1.0a");
    //
    SetOperatingLevel(VT500_S7C); // default
    //
    AddVT520ControlSeqs(this->mSeqsFactory);
    //
    LoadDefaultModes();
    //
    InstallFunctions();
}

void SerialConnVT520::LoadDefaultModes()
{
}

void SerialConnVT520::InstallFunctions()
{
    mFunctions[DECAC] = [=](const std::string& p) { ProcessDECAC(p); };
    mFunctions[DECRQM_ANSI] = [=](const std::string& p) { ProcessDECRQM_ANSI(p); };
    mFunctions[DECRQM_DECP] = [=](const std::string& p) { ProcessDECRQM_DECP(p); };
    mFunctions[DECST8C] = [=](const std::string& p) { ProcessDECST8C(p); };
}

void SerialConnVT520::ProcessDECST8C(const std::string&)
{
    this->mTabWidth = 8;
}

#define DO_SET_CHARSET(g)                                 \
    do {                                                  \
        if (p == "\">") {                                 \
            mCharsets[g] = CS_GREEK;                      \
        } else if (p == "%=") {                           \
            mCharsets[g] = CS_HEBREW;                     \
        } else if (p == "%2") {                           \
            mCharsets[g] = CS_TURKISH;                    \
        } else if (p == "%3") {                           \
            mCharsets[g] = CS_SCS;                        \
        } else if (p == "&5") {                           \
            mCharsets[g] = CS_RUSSIAN;                    \
        } else                                            \
            SerialConnVT420::ProcessG##g##_CS(p);         \
    } while (0)
//
#define DO_SET_CHARSET96(g)                               \
    do {                                                  \
        if (p == "B") {                                   \
            mCharsets[g] = CS_ISO_LATIN2_SUPPLEMENTAL;    \
        } else if (p == "F") {                            \
            mCharsets[g] = CS_ISO_GREEK_SUPPLEMENTAL;     \
        } else if (p == "H") {                            \
            mCharsets[g] = CS_ISO_HEBREW_SUPPLEMENTAL;    \
        } else if (p == "L") {                            \
            mCharsets[g] = CS_ISO_LATIN_CYRILLIC;         \
        } else if (p == "M") {                            \
            mCharsets[g] = CS_ISO_LATIN5_SUPPLEMENTAL;    \
        } else                                            \
            SerialConnVT420::ProcessG##g##_CS96(p);       \
    } while (0)
void SerialConnVT520::ProcessG0_CS(const std::string& p)
{
    DO_SET_CHARSET(0);
}
void SerialConnVT520::ProcessG1_CS(const std::string& p)
{
    DO_SET_CHARSET(1);
}
void SerialConnVT520::ProcessG2_CS(const std::string& p)
{
    DO_SET_CHARSET(2);
}
void SerialConnVT520::ProcessG3_CS(const std::string& p)
{
    DO_SET_CHARSET(3);
}
void SerialConnVT520::ProcessG1_CS96(const std::string& p)
{
    DO_SET_CHARSET96(1);
}
void SerialConnVT520::ProcessG2_CS96(const std::string& p)
{
    DO_SET_CHARSET96(2);
}
void SerialConnVT520::ProcessG3_CS96(const std::string& p)
{
    DO_SET_CHARSET96(3);
}

void SerialConnVT520::ProcessDECRQM_ANSI(const std::string& p)
{
    int ps = atoi(p.data());
    std::string rsp = std::string("\033[") + std::to_string(ps) + ";";
    switch (ps) {
    case 1:
    case 5:
    case 7:
    case 10:
    case 11:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
        // These modes were permanently reset
        Put(rsp + "4$y");
        break;
    default:
        Put(rsp + (mModes.GetAnsiMode(ps, 0) ? "1" : "2") + "$y");
        break;
    }
}
void SerialConnVT520::ProcessDECRQM_DECP(const std::string& p)
{
    int ps = atoi(p.data());
    std::string rsp = std::string("\033[?") + std::to_string(ps) + ";";
    switch (ps) {
    // permanently reset
    case 34: Put(rsp + "4$y"); break;
    default:
        Put(rsp + (mModes.GetAnsiMode(ps, 0) ? "1" : "2") + "$y");
        break;
    }
}

void SerialConnVT520::ProcessDECAC(const std::string& p)
{
    int ps[3] = {0}, idx = 0;
    SplitString(p.data(), ";", [=, &ps, &idx](const char* token) {
        if (idx < 3)
            ps[idx] = atoi(token);
        idx++;
    });
    if (idx == 3) {
        switch (ps[0]) {
        case 1: // normal text
            this->mColorTbl.SetColor(VTColorTable::kColorId_Texts, this->mColorTbl.GetColor(ps[1]));
            this->mColorTbl.SetColor(VTColorTable::kColorId_Paper, this->mColorTbl.GetColor(ps[2]));
            break;
        case 2: // window frame
            break;
        }
    }
}

void SerialConnVT520::ProcessDA(const std::string& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 0:
        // 1   132 columns
        // 2   Printer port
        // 6   Selective erase
        // 7   Soft character set
        // 8   User defined keys
        // 9   National replacement character sets
        // 12  Serbo-Croatian
        // 15  Technical character sets
        // 18  Windowing capability
        // 19  Sessions
        // 21  Horizontal scrolling
        // 22  Color
        // 23  Greek
        // 24  Turkish
        // 42  ISO Latin-2
        // 44  PCTerm
        // 45  Soft key mapping
        // 46  ASCII terminal emulation
        Put("\033[?7;9;22;23;24;42;44;45;46c");
        break;
    default:
        SerialConnVT420::ProcessDA(p);
        break;
    }
}

void SerialConnVT520::ProcessDECSCL(const std::string& p)
{
    int idx = 0;
    int ps[2] = {0, 0};
    SplitString(p.data(), ";", [=, &idx, &ps](const char* token) {
        if (idx < 2)
            ps[idx] = atoi(token);
    });
    int level = 0;
    if (ps[0] == 65) {
        level = VT500_S7C;
        if (ps[1] == 0 || ps[1] == 2)
            level |= VTFLG_S8C;
        //
        SetOperatingLevel(level);
    } else
        SerialConnVT420::ProcessDECSCL(p);
}

void SerialConnVT520::ProcessSecondaryDA(const std::string& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 0:
        // CSI > 64; Pv; 1 c , VT520
        // Pv - firmware version.
        GetIo()->Write("\033[>64;20;1c");
        break;
    default:
        SerialConnVT420::ProcessSecondaryDA(p);
    }
}
