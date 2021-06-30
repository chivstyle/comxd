/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT420.h"
#include "VT420ControlSeq.h"
#include "VT420Charset.h"
#include "ConnFactory.h"

REGISTER_CONN_INSTANCE("vt420 by chiv", "vt420", SerialConnVT420);

using namespace Upp;

SerialConnVT420::SerialConnVT420(std::shared_ptr<SerialIo> io)
    : SerialConnVT320(io)
    , SerialConnVT(io)
{
    this->mOperatingLevel = VT400_S7C; // default
    //
    AddVT420ControlSeqs(this->mSeqsFactory);
    //
    InstallFunctions();
}

void SerialConnVT420::InstallFunctions()
{
    mFunctions[TertiaryDA] = [=](const std::string_view& p) { ProcessTertiaryDA(p); };
}

void SerialConnVT420::ProcessDECFI(const std::string_view& p)
{
    mPx -= mFontW;
}
void SerialConnVT420::ProcessDECBI(const std::string_view& p)
{
    mPx += mFontW;
}

void SerialConnVT420::ProcessDECSCL(const std::string_view& p)
{
    if (p == "64" || p == "64;0" || p == "64;2" || \
        p == "63" || p == "63;0" || p == "63;2" || \
        p == "62" || p == "62;0" || p == "62;2") {
        mOperatingLevel = VT400_S8C;
    } else if (p == "64;1" || p == "63;1" || p == "62;1") {
        mOperatingLevel = VT400_S7C;
    } else {
        SerialConnVT320::ProcessDECSCL(p);
    }
}

void SerialConnVT420::SetHostToS7C()
{
    GetIo()->Write("\E F");
}

void SerialConnVT420::SetHostToS8C()
{
    GetIo()->Write("\E G");
}

void SerialConnVT420::ProcessDECSM(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 81:
        mModes.DECKPM = 1;
        break;
    default: SerialConnVT320::ProcessDECSM(p);
    }
}
void SerialConnVT420::ProcessDECRM(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 81:
        mModes.DECKPM = 0;
        break;
    default: SerialConnVT320::ProcessDECRM(p);
    }
}

void SerialConnVT420::ProcessDA(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 0:
        // CSI ? Psc; Ps1; ...Psn c
        // Psc, 62,63,64 - level 4, 61 - level 1
        // Ps1...Psn
        // 1 - 132 columns, 2 - printer port, 6 - selective erase
        // 7 - soft character set, 8 - user-defined keys
        // 9 - NRC sets, 15 - DEC technical set
        // 18 - user windows, 19 - two sessions
        // 21 - horizontal scrolling
        GetIo()->Write("\E[?62,63,64;6;9;15;18c");
        break;
    default:SerialConnVT320::ProcessDA(p);
    }
}

void SerialConnVT420::ProcessSecondaryDA(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 0:
        // CSI > 41; Pv; 0 c
        // Pv - firmware version.
        GetIo()->Write("\E[>41;v1.0a;0c");
        break;
    default:SerialConnVT320::ProcessSecondaryDA(p);
    }
}

void SerialConnVT420::ProcessTertiaryDA(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 0:
        // DCS!|D...D ST
        GetIo()->Write("\E\x50!|0\E\x5c");
        break;
    default:break;
    }
}
// TODO: https://vt100.net/docs/vt420-uu/chapter9.html, 12 VT420 Reports
void SerialConnVT420::ProcessDECDSR(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 6: if (1) {
        std::string rsp = std::string("\E[?") + \
            std::to_string(mVy) + ";" + \
            std::to_string(mVx) + ";0R";
        GetIo()->Write(rsp);
    } break;
    default: SerialConnVT320::ProcessDECDSR(p);
    }
}
