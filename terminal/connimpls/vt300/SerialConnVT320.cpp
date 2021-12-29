/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT320.h"
#include "ConnFactory.h"
#include "VT320Charset.h"
#include "VT320ControlSeq.h"

//REGISTER_CONN_INSTANCE("vt320 by chiv", "vt320", SerialConnVT320);

using namespace Upp;

SerialConnVT320::SerialConnVT320(std::shared_ptr<SerialIo> io)
    : SerialConnVT220(io)
    , SerialConnVT(io)
{
    SetConnDescription("vt320 emulator, by chiv, v1.0a");
    // vt100 supports G0,G1, vt200 supports G2,G3
    mCharsets[2] = CS_DEFAULT;
    mCharsets[3] = CS_DEFAULT;
    //
    SetOperatingLevel(VT300_S7C);
    //
    AddVT320ControlSeqs(this->mSeqsFactory);
    //
    LoadDefaultModes();
    //
    InstallFunctions();
}

void SerialConnVT320::LoadDefaultModes()
{
}

void SerialConnVT320::ProcessDECSCL(const std::string& p)
{
    int idx = 0;
    int ps[2] = {0, 0};
    SplitString(p.data(), ";", [=, &idx, &ps](const char* token) {
        if (idx < 2)
            ps[idx] = atoi(token);
        idx++;
    });
    int level = 0;
    if (ps[0] == 65) {
        level = VT300_S7C;
        if (ps[1] == 0 || ps[1] == 2)
            level |= VTFLG_S8C;
        //
        SetOperatingLevel(level);
    } else
        SerialConnVT220::ProcessDECSCL(p);
}

void SerialConnVT320::InstallFunctions()
{
    mFunctions[DECSASD] = [=](const std::string& p) { ProcessDECSASD(p); };
    mFunctions[DECSSDT] = [=](const std::string& p) { ProcessDECSSDT(p); };
    mFunctions[DECRQTSR] = [=](const std::string& p) { ProcessDECRQTSR(p); };
    mFunctions[DECRQPSR] = [=](const std::string& p) { ProcessDECRQPSR(p); };
    mFunctions[DECRQM] = [=](const std::string& p) { ProcessDECRQM(p); };
    mFunctions[ANSIRQM] = [=](const std::string& p) { ProcessANSIRQM(p); };
    mFunctions[DECRPM] = [=](const std::string& p) { ProcessDECRPM(p); };
    mFunctions[DECRQUPSS] = [=](const std::string& p) { ProcessDECRQUPSS(p); };
    mFunctions[G1_CS96] = [=](const std::string& p) { ProcessG1_CS96(p); };
    mFunctions[G2_CS96] = [=](const std::string& p) { ProcessG2_CS96(p); };
    mFunctions[G3_CS96] = [=](const std::string& p) { ProcessG3_CS96(p); };
}
//
#define DO_SET_CHARSET(g)                                 \
    do {                                                  \
        if (p == "%5") {                                  \
            mCharsets[g] = CS_DEC_SUPPLEMENTAL_VT320;     \
        } else if (p == "`" || p == "E" || p == "6") {    \
            mCharsets[g] = CS_DANISH_VT320;               \
        } else if (p == "%6") {                           \
            mCharsets[g] = CS_PORTUGUESE;                 \
        } else if (p == "<") {                            \
            mCharsets[g] = CS_USER_PREFERED_SUPPLEMENTAL; \
        } else                                            \
            SerialConnVT220::ProcessG##g##_CS(p);         \
    } while (0)
//
#define DO_SET_CHARSET96(g) do { } while (0)
//
void SerialConnVT320::ProcessG0_CS(const std::string& p)
{
    DO_SET_CHARSET(0);
}
void SerialConnVT320::ProcessG1_CS(const std::string& p)
{
    DO_SET_CHARSET(1);
}
void SerialConnVT320::ProcessG2_CS(const std::string& p)
{
    DO_SET_CHARSET(2);
}
void SerialConnVT320::ProcessG3_CS(const std::string& p)
{
    DO_SET_CHARSET(3);
}
void SerialConnVT320::ProcessG1_CS96(const std::string&)
{
    DO_SET_CHARSET96(1);
}
void SerialConnVT320::ProcessG2_CS96(const std::string&)
{
    DO_SET_CHARSET96(2);
}
void SerialConnVT320::ProcessG3_CS96(const std::string&)
{
    DO_SET_CHARSET96(3);
}
//
void SerialConnVT320::ProcessDECSASD(const std::string&)
{
}
void SerialConnVT320::ProcessDECSSDT(const std::string&)
{
}
void SerialConnVT320::ProcessDECRQTSR(const std::string&)
{
}
void SerialConnVT320::ProcessDECRQPSR(const std::string&)
{
}
void SerialConnVT320::ProcessDECRQM(const std::string&)
{
}
void SerialConnVT320::ProcessANSIRQM(const std::string&)
{
}
void SerialConnVT320::ProcessDECRPM(const std::string&)
{
}
void SerialConnVT320::ProcessDECRQUPSS(const std::string&)
{
}
//
void SerialConnVT320::ProcessDA(const std::string& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 0:
        // level3 (VT300), NRC sets
        Put("\033[62,63;9c");
        break;
    }
}
void SerialConnVT320::ProcessSecondaryDA(const std::string& p)
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

void SerialConnVT320::ProcessDECSEL(const std::string& p)
{
    if (mCursorData.SelectiveErase)
        return;
    int ps = atoi(p.data());
    switch (ps) {
    case 0:
        if (1) {
            VTLine& vline = mLines[mVy];
            for (int i = mVx; i < (int)vline.size(); ++i) {
                vline[i] = ' ';
            }
        }
        break;
    case 1:
        if (1) {
            VTLine& vline = mLines[mVy];
            for (int i = 0; i <= mVx && i < (int)vline.size(); ++i) {
                vline[i] = ' ';
            }
        }
        break;
    case 2:
        if (1) {
            VTLine& vline = mLines[mVy];
            for (int i = 0; i < (int)vline.size(); ++i) {
                vline[i] = ' ';
            }
        }
        break;
    }
}
void SerialConnVT320::ProcessDECSED(const std::string& p)
{
    if (!mCursorData.SelectiveErase)
        return;
    int ps = atoi(p.data());
    switch (ps) {
    case 0:
        if (1) {
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
        break;
    case 1:
        if (1) {
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
        }
        break;
    case 2:
        if (1) {
            for (int y = 0; y < (int)mLines.size(); ++y) {
                VTLine& vline = mLines[y];
                for (int i = 0; i < (int)vline.size(); ++i) {
                    vline[i] = ' ';
                }
            }
        }
        break;
    }
}
