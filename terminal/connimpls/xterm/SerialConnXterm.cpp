/*!
// (c) 2021 chiv
//
*/
#include "SerialConnXterm.h"
#include "XtermControlSeq.h"
#include "XtermCharset.h"
#include "ConnFactory.h"

REGISTER_CONN_INSTANCE("xterm by chiv", "xterm", SerialConnXterm);

using namespace Upp;

SerialConnXterm::SerialConnXterm(std::shared_ptr<SerialIo> io)
    : SerialConnVT520(io)
    , SerialConnVT(io)
{
    AddXtermControlSeqs(this->mSeqsFactory);
    // take a snap the current screen
    this->SaveScr(mAlternateScr);
    //
    InstallFunctions();
}

void SerialConnXterm::InstallFunctions()
{
    mFunctions[XTPUSHCOLORS] = [=](const std::string_view& p) { ProcessXTPUSHCOLORS(p); };
    mFunctions[XTPOPCOLORS] = [=](const std::string_view& p) { ProcessXTPOPCOLORS(p); };
    mFunctions[XTSMGRAPHICS] = [=](const std::string_view& p) { ProcessXTSMGRAPHICS(p); };
    mFunctions[XTHIMOUSE] = [=](const std::string_view& p) { ProcessXTHIMOUSE(p); };
    mFunctions[XTRMTITLE] = [=](const std::string_view& p) { ProcessXTRMTITLE(p); };
    mFunctions[XTMODKEYS] = [=](const std::string_view& p) { ProcessXTMODKEYS(p); };
    mFunctions[XTSMPOINTER] = [=](const std::string_view& p) { ProcessXTSMPOINTER(p); };
    mFunctions[XTPUSHSGR] = [=](const std::string_view& p) { ProcessXTPUSHSGR(p); };
    mFunctions[XTPOPSGR] = [=](const std::string_view& p) { ProcessXTPOPSGR(p); };
    mFunctions[XTVERSION] = [=](const std::string_view& p) { ProcessXTVERSION(p); };
    mFunctions[XTRESTORE] = [=](const std::string_view& p) { ProcessXTRESTORE(p); };
    mFunctions[XTSAVE] = [=](const std::string_view& p) { ProcessXTSAVE(p); };
    mFunctions[XTWINOPS] = [=](const std::string_view& p) { ProcessXTWINOPS(p); };
    mFunctions[XTSMTITLE] = [=](const std::string_view& p) { ProcessXTSMTITLE(p); };
    mFunctions[XTCHECKSUM] = [=](const std::string_view& p) { ProcessXTCHECKSUM(p); };
}

void SerialConnXterm::ProcessXTPUSHCOLORS(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTPOPCOLORS(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTSMGRAPHICS(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTHIMOUSE(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTRMTITLE(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTMODKEYS(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTSMPOINTER(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTPUSHSGR(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTPOPSGR(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTVERSION(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTRESTORE(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTSAVE(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTWINOPS(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTSMTITLE(const std::string_view&)
{
}
void SerialConnXterm::ProcessXTCHECKSUM(const std::string_view&)
{
}

void SerialConnXterm::ProcessDECSM(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 1048:
        SerialConnVT520::ProcessDECSC(p);
        break;
    case 47:
    case 1047:
    case 1049:
        // switch to alternate screen
        if (mModes.UseAlternateScr == 0) {
            this->SwapScr(mAlternateScr);
            mModes.UseAlternateScr = 1;
            if (pn == 1049) {
                Clear();
            }
        }
        break;
    default:
        SerialConnVT520::ProcessDECSM(p);
        break;
    }
}

void SerialConnXterm::ProcessDECRM(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 1048:
        SerialConnVT520::ProcessDECRC(p);
        break;
    case 47:
    case 1047:
    case 1049:
        // switch to main screen
        if (mModes.UseAlternateScr == 1) {
            this->SwapScr(mAlternateScr);
            mModes.UseAlternateScr = 0;
        }
        break;
    default:
        SerialConnVT520::ProcessDECRM(p);
        break;
    }
}
