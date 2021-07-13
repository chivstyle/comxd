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
    InstallFunctions();
}

void SerialConnVT520::InstallFunctions()
{
}

void SerialConnVT520::ProcessDECSM(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 95:
        mModes.DECNCSM = 1;
        break;
    case 96:
        mModes.DECRLCM = 1;
        break;
    case 98:
        mModes.DECARSM = 1;
        break;
    case 99:
        mModes.DECMCM = 1;
        break;
    case 100:
        mModes.DECAAM = 1;
        break;
    case 101:
        mModes.DECCANSM = 1;
        break;
    case 102:
        mModes.DECNULM = 1;
        break;
    case 103:
        mModes.DECHDPXM = 1;
        break;
    case 104:
        mModes.DECESKM = 1;
        break;
    case 106:
        mModes.DECOSCNM = 1;
        break;
    case 108:
        mModes.DECNUMLK = 1;
        break;
    case 109:
        mModes.DECCAPSLK = 1;
        break;
    case 110:
        mModes.DECKLHIM = 1;
        break;
    case 111:
        mModes.DECFWM = 1;
        break;
    case 112:
        mModes.DECRPL = 1;
        break;
    case 113:
        mModes.DECHWUM = 1;
        break;
    case 114:
        mModes.DECATCUM = 1;
        break;
    case 115:
        mModes.DECATCBM = 1;
        break;
    case 116:
        mModes.DECBBSM = 1;
        break;
    case 117:
        mModes.DECECM = 1;
        break;
    default:
        SerialConnVT420::ProcessDECSM(p);
        break;
    }
}
void SerialConnVT520::ProcessDECRM(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 95:
        mModes.DECNCSM = 0;
        break;
    case 96:
        mModes.DECRLCM = 0;
        break;
    case 98:
        mModes.DECARSM = 0;
        break;
    case 99:
        mModes.DECMCM = 0;
        break;
    case 100:
        mModes.DECAAM = 0;
        break;
    case 101:
        mModes.DECCANSM = 0;
        break;
    case 102:
        mModes.DECNULM = 0;
        break;
    case 103:
        mModes.DECHDPXM = 0;
        break;
    case 104:
        mModes.DECESKM = 0;
        break;
    case 106:
        mModes.DECOSCNM = 0;
        break;
    case 108:
        mModes.DECNUMLK = 0;
        break;
    case 109:
        mModes.DECCAPSLK = 0;
        break;
    case 110:
        mModes.DECKLHIM = 0;
        break;
    case 111:
        mModes.DECFWM = 0;
        break;
    case 112:
        mModes.DECRPL = 0;
        break;
    case 113:
        mModes.DECHWUM = 0;
        break;
    case 114:
        mModes.DECATCUM = 0;
        break;
    case 115:
        mModes.DECATCBM = 0;
        break;
    case 116:
        mModes.DECBBSM = 0;
        break;
    case 117:
        mModes.DECECM = 0;
        break;
    default:
        SerialConnVT420::ProcessDECRM(p);
        break;
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

void SerialConnVT520::ProcessSM(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 97:
        mModes.DECCRTSM = 1;
        break;
    default:
        SerialConnVT420::ProcessSM(p);
        break;
    }
}
void SerialConnVT520::ProcessRM(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 97:
        mModes.DECCRTSM = 0;
        break;
    default:
        SerialConnVT420::ProcessRM(p);
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
