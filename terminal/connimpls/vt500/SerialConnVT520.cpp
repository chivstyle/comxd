/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT520.h"
#include "ConnFactory.h"
#include "VT520Charset.h"
#include "VT520ControlSeq.h"

REGISTER_CONN_INSTANCE("vt520 by chiv", "vt520", SerialConnVT520);

using namespace Upp;

SerialConnVT520::SerialConnVT520(std::shared_ptr<SerialIo> io)
    : SerialConnVT420(io)
    , SerialConnVT(io)
{
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
    mFunctions[DECAC] = [=](const std::string_view& p) { ProcessDECAC(p); };
    mFunctions[DECRQM_ANSI] = [=](const std::string_view& p) { ProcessDECRQM_ANSI(p); };
    mFunctions[DECRQM_DECP] = [=](const std::string_view& p) { ProcessDECRQM_DECP(p); };
}

void SerialConnVT520::ProcessDECRQM_ANSI(const std::string_view& p)
{
    int ps = atoi(p.data());
    std::string rsp = std::string("\E[") + std::to_string(ps) + ";";
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
void SerialConnVT520::ProcessDECRQM_DECP(const std::string_view& p)
{
    int ps = atoi(p.data());
    std::string rsp = std::string("\E[?") + std::to_string(ps) + ";";
    switch (ps) {
    // permanently reset
    case 34: Put(rsp + "4$y"); break;
    default:
        Put(rsp + (mModes.GetAnsiMode(ps, 0) ? "1" : "2") + "$y");
        break;
    }
}

void SerialConnVT520::ProcessDECAC(const std::string_view& p)
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

void SerialConnVT520::ProcessDA(const std::string_view& p)
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
        Put("\E[?7;9;22;23;24;42;44;45;46c");
        break;
    default:
        SerialConnVT420::ProcessDA(p);
        break;
    }
}

void SerialConnVT520::ProcessDECSCL(const std::string_view& p)
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

void SerialConnVT520::ProcessSecondaryDA(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 0:
        // CSI > 64; Pv; 1 c , VT520
        // Pv - firmware version.
        GetIo()->Write("\E[>64;20;1c");
        break;
    default:
        SerialConnVT420::ProcessSecondaryDA(p);
    }
}
