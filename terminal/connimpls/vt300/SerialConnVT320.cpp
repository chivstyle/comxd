/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT320.h"
#include "VT320ControlSeq.h"
#include "VT320Charset.h"
#include "ConnFactory.h"

REGISTER_CONN_INSTANCE("vt320 by chiv", "vt320", SerialConnVT320);

using namespace Upp;

SerialConnVT320::SerialConnVT320(std::shared_ptr<SerialIo> io)
    : SerialConnVT220(io)
    , SerialConnVT(io)
{
    // vt100 supports G0,G1, vt200 supports G2,G3
    mCharsets[2] = CS_DEFAULT;
    mCharsets[3] = CS_DEFAULT;
    mOperatingLevel = VT300_S7C;
    //
    AddVT320ControlSeqs(this->mSeqsFactory);
    //
    InstallFunctions();
}

void SerialConnVT320::ProcessDECSCL(const std::string_view& p)
{
    if (p == "63" || p == "63;0" || p == "63;2" || p == "62" || \
        p == "62;0" || p == "62;2") {
        mOperatingLevel = VT300_S8C;
    } else if (p == "63;1" || p == "62;1") {
        mOperatingLevel = VT300_S7C;
    } else {
        SerialConnVT220::ProcessDECSCL(p);
    }
}

void SerialConnVT320::InstallFunctions()
{
    mFunctions[DECSASD]   = [=](const std::string_view& p) { ProcessDECSASD(p); };
    mFunctions[DECSSDT]   = [=](const std::string_view& p) { ProcessDECSSDT(p); };
    mFunctions[DECRQTSR]  = [=](const std::string_view& p) { ProcessDECRQTSR(p); };
    mFunctions[DECRQPSR]  = [=](const std::string_view& p) { ProcessDECRQPSR(p); };
    mFunctions[DECRQM]    = [=](const std::string_view& p) { ProcessDECRQM(p); };
    mFunctions[ANSIRQM]   = [=](const std::string_view& p) { ProcessANSIRQM(p); };
    mFunctions[DECRPM]    = [=](const std::string_view& p) { ProcessDECRPM(p); };
    mFunctions[DECRQUPSS] = [=](const std::string_view& p) { ProcessDECRQUPSS(p); };
}
//
#define DO_SET_CHARSET(g) do { \
    if (p == "%5") { \
        mCharsets[g] = CS_DEC_SUPPLEMENTAL_VT320; \
    } else if (p == "`" || p == "E" || p == "6") { \
        mCharsets[g] = CS_DANISH_VT320; \
    } else if (p == "%6") { \
        mCharsets[g] = CS_PORTUGUESE; \
    } else if (p == "<") { \
        mCharsets[g] = CS_USER_PREFERED_SUPPLEMENTAL; \
    } else \
        SerialConnVT220::ProcessG##g##_CS(p); \
} while (0)
//
void SerialConnVT320::ProcessG0_CS(const std::string_view& p)
{
    DO_SET_CHARSET(0);
}
void SerialConnVT320::ProcessG1_CS(const std::string_view& p)
{
    DO_SET_CHARSET(1);
}
void SerialConnVT320::ProcessG2_CS(const std::string_view& p)
{
    DO_SET_CHARSET(2);
}
void SerialConnVT320::ProcessG3_CS(const std::string_view& p)
{
    DO_SET_CHARSET(3);
}
//
void SerialConnVT320::ProcessDECSASD(const std::string_view&)
{
}
void SerialConnVT320::ProcessDECSSDT(const std::string_view&)
{
}
void SerialConnVT320::ProcessDECRQTSR(const std::string_view&)
{
}
void SerialConnVT320::ProcessDECRQPSR(const std::string_view&)
{
}
void SerialConnVT320::ProcessDECRQM(const std::string_view&)
{
}
void SerialConnVT320::ProcessANSIRQM(const std::string_view&)
{
}
void SerialConnVT320::ProcessDECRPM(const std::string_view&)
{
}
void SerialConnVT320::ProcessDECRQUPSS(const std::string_view&)
{
}
//
void SerialConnVT320::ProcessDA(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 0:
        // level3 (VT300), NRC sets
        Put("\033[62,63;9c");
        break;
    }
}
void SerialConnVT320::ProcessSecondaryDA(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 0:
        // vt320 CSI > Pp ; Pv ; Po c
        Put("\033[24;20;0c");
        break;
    default:
        SerialConnVT220::ProcessSecondaryDA(p);
    }
}

void SerialConnVT320::ProcessDECSM(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 68: this->mModes.DECKBUM = 1; break;
    default: SerialConnVT220::ProcessDECSM(p); break;
    }
}
void SerialConnVT320::ProcessDECRM(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 68: this->mModes.DECKBUM = 0; break;
    default: SerialConnVT220::ProcessDECRM(p); break;
    }
}

void SerialConnVT320::ProcessDECSEL(const std::string_view& p)
{
    if (mSelectiveErase == false) return;
    int ps = atoi(p.data());
    switch (ps) {
    case 0: if (1) {
        VTLine& vline = mLines[mVy];
        for (int i = mVx; i < (int)vline.size(); ++i) {
            vline[i] = ' ';
        }
    } break;
    case 1: if (1) {
        VTLine& vline = mLines[mVy];
        for (int i = 0; i <= mVx && i < (int)vline.size(); ++i) {
            vline[i] = ' ';
        }
    } break;
    case 2: if (1) {
        VTLine& vline = mLines[mVy];
        for (int i = 0; i < (int)vline.size(); ++i) {
            vline[i] = ' ';
        }
    } break;
    }
}
void SerialConnVT320::ProcessDECSED(const std::string_view& p)
{
    if (mSelectiveErase == false) return;
    int ps = atoi(p.data());
    switch (ps) {
    case 0: if (1) {
        VTLine& vline = mLines[mVy];
        for (int i = mVx; i < (int)vline.size(); ++i) {
            vline[i] = ' ';
        }
        for (int y = mVy+1; y < (int)mLines.size(); ++y) {
            for (int x = 0; x < (int)mLines[y].size(); ++x) {
                mLines[y][x] = ' ';
            }
        }
    } break;
    case 1: if (1) {
        VTLine& vline = mLines[mVy];
        for (int i = 0; i <= mVx && i < (int)vline.size(); ++i) {
            vline[i] = ' ';
        }
        for (int y = 0; y < mVy; ++y) {
            VTLine& vline = mLines[y];
            for (int i = 0; i < (int)vline.size(); ++i) {
                vline[i] = ' ';
            }
        }
    } break;
    case 2: if (1) {
        for (int y = 0; y < (int)mLines.size(); ++y) {
            VTLine& vline = mLines[y];
            for (int i = 0; i < (int)vline.size(); ++i) {
                vline[i] = ' ';
            }
        }
    } break;
    }
}
