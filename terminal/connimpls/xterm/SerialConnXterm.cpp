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
    , mCurrentScr(SC_Main)
{
    AddXtermControlSeqs(this->mSeqsFactory);
    // take a snap the current screen
    this->SaveScr(mAlternateScr);
    //
    InstallFunctions();
}

void SerialConnXterm::InstallFunctions()
{
}

void SerialConnXterm::ProcessDECSM(const std::string_view& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 1049:
        // switch to alternate screen
        if (mCurrentScr == SC_Main) {
            this->SwapScr(mAlternateScr);
            mCurrentScr = SC_Alternate;
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
    case 1049:
        // switch to main screen
        if (mCurrentScr == SC_Alternate) {
            this->SwapScr(mAlternateScr);
            mCurrentScr = SC_Main;
        }
        break;
    default:
        SerialConnVT520::ProcessDECRM(p);
        break;
    }
}
