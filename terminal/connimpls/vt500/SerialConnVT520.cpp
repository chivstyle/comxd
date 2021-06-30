/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT520.h"
#include "VT520ControlSeq.h"
#include "VT520Charset.h"
#include "ConnFactory.h"

REGISTER_CONN_INSTANCE("vt520 by chiv", "vt520", SerialConnVT520);

using namespace Upp;

SerialConnVT520::SerialConnVT520(std::shared_ptr<SerialIo> io)
    : SerialConnVT420(io)
    , SerialConnVT(io)
{
    this->mOperatingLevel = VT500_S7C; // default
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
    case 95: mModes.DECNCSM = 1; break;
    case 96: mModes.DECRLCM = 1; break;
    case 98: mModes.DECARSM = 1; break;
    case 99: mModes.DECMCM = 1; break;
    case 100: mModes.DECAAM = 1; break;
    case 101: mModes.DECCANSM = 1; break;
    case 102: mModes.DECNULM = 1; break;
    case 103: mModes.DECHDPXM = 1; break;
    case 104: mModes.DECESKM = 1; break;
    case 106: mModes.DECOSCNM = 1; break;
    case 108: mModes.DECNUMLK = 1; break;
    case 109: mModes.DECCAPSLK = 1; break;
    case 110: mModes.DECKLHIM = 1; break;
    case 111: mModes.DECFWM = 1; break;
    case 112: mModes.DECRPL = 1; break;
    case 113: mModes.DECHWUM = 1; break;
    case 114: mModes.DECATCUM = 1; break;
    case 115: mModes.DECATCBM = 1; break;
    case 116: mModes.DECBBSM = 1; break;
    case 117: mModes.DECECM = 1; break;
    default:
        SerialConnVT420::ProcessDECSM(p);
    }
}
void SerialConnVT520::ProcessDECRM(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 95: mModes.DECNCSM = 0; break;
    case 96: mModes.DECRLCM = 0; break;
    case 98: mModes.DECARSM = 0; break;
    case 99: mModes.DECMCM = 0; break;
    case 100: mModes.DECAAM = 0; break;
    case 101: mModes.DECCANSM = 0; break;
    case 102: mModes.DECNULM = 0; break;
    case 103: mModes.DECHDPXM = 0; break;
    case 104: mModes.DECESKM = 0; break;
    case 106: mModes.DECOSCNM = 0; break;
    case 108: mModes.DECNUMLK = 0; break;
    case 109: mModes.DECCAPSLK = 0; break;
    case 110: mModes.DECKLHIM = 0; break;
    case 111: mModes.DECFWM = 0; break;
    case 112: mModes.DECRPL = 0; break;
    case 113: mModes.DECHWUM = 0; break;
    case 114: mModes.DECATCUM = 0; break;
    case 115: mModes.DECATCBM = 0; break;
    case 116: mModes.DECBBSM = 0; break;
    case 117: mModes.DECECM = 0; break;
    default:
        SerialConnVT420::ProcessDECRM(p);
    }
}
void SerialConnVT520::ProcessSM(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 97: mModes.DECCRTSM = 1; break;
    default:
        SerialConnVT420::ProcessSM(p);
    }
}
void SerialConnVT520::ProcessRM(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 97: mModes.DECCRTSM = 0; break;
    default:
        SerialConnVT420::ProcessRM(p);
    }
}

void SerialConnVT520::ProcessDECSCL(const std::string_view& p)
{
    if (p == "65" || p == "65;0" || p == "65;2" || \
        p == "64" || p == "64;0" || p == "64;2" || \
        p == "63" || p == "63;0" || p == "63;2" || \
        p == "62" || p == "62;0" || p == "62;2") {
        mOperatingLevel = VT500_S8C;
    } else if (p == "64;1" || p == "63;1" || p == "62;1") {
        mOperatingLevel = VT500_S7C;
    } else {
        SerialConnVT420::ProcessDECSCL(p);
    }
}

void SerialConnVT520::ProcessSecondaryDA(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 0:
        // CSI > 64; Pv; 0 c , VT520
        // Pv - firmware version.
        GetIo()->Write("\E[>64;v1.0a;0c");
        break;
    default:SerialConnVT420::ProcessSecondaryDA(p);
    }
}
