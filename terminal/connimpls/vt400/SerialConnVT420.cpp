/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT420.h"
#include "ConnFactory.h"
#include "VT420Charset.h"
#include "VT420ControlSeq.h"

//REGISTER_CONN_INSTANCE("vt420 by chiv", "vt420", SerialConnVT420);

using namespace Upp;

SerialConnVT420::SerialConnVT420(std::shared_ptr<SerialIo> io)
    : SerialConnVT320(io)
    , SerialConnVT(io)
{
    SetConnDescription("vt420 emulator, by chiv, v1.0a");
    //
    SetOperatingLevel(VT400_S7C); // default
    //
    AddVT420ControlSeqs(this->mSeqsFactory);
    //
    LoadDefaultModes();
    //
    InstallFunctions();
}

void SerialConnVT420::LoadDefaultModes()
{
}

void SerialConnVT420::InstallFunctions()
{
    mFunctions[TertiaryDA] = [=](const std::string& p) { ProcessTertiaryDA(p); };
    mFunctions[DECDC] = [=](const std::string& p) { ProcessDECDC(p); };
}

void SerialConnVT420::ProcessDECFI(const std::string& p)
{
    mPx -= mFontW;
}
void SerialConnVT420::ProcessDECBI(const std::string& p)
{
    mPx += mFontW;
}

void SerialConnVT420::ProcessDECDC(const std::string& p)
{
    int pn = atoi(p.data());
    if (pn <= 0) pn = 1;
    int top = mScrollingRegion.Top;
    int bot = mScrollingRegion.Bottom;
    if (bot < 0)
        bot = (int)mLines.size() - 1;
    for (int k = top; k <= bot; ++k) {
        if (pn <= (int)mLines[k].size()) {
            mLines[k].erase(mLines[k].begin() + pn - 1);
            mLines[k].push_back(mBlankChar);
        }
    }
}
void SerialConnVT420::ProcessDECIC(const std::string& p)
{
    int pn = atoi(p.data());
    if (pn <= 0) pn = 1;
    int top = mScrollingRegion.Top;
    int bot = mScrollingRegion.Bottom;
    if (bot < 0)
        bot = (int)mLines.size() - 1;
    for (int k = top; k <= bot; ++k) {
        if (pn <= (int)mLines[k].size()) {
            mLines[k].insert(mLines[k].begin() + pn - 1, mBlankChar);
        }
    }
}

void SerialConnVT420::ProcessDECSCL(const std::string& p)
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
        level = VT400_S7C;
        if (ps[1] == 0 || ps[1] == 2)
            level |= VTFLG_S8C;
        //
        SetOperatingLevel(level);
    } else
        SerialConnVT320::ProcessDECSCL(p);
}

void SerialConnVT420::SetHostToS7C()
{
    Put("\033 F");
}

void SerialConnVT420::SetHostToS8C()
{
    Put("\033 G");
}

#define DO_SET_CHARSET96(g)                               \
    do {                                                  \
        if (p == "A") {                                   \
            mCharsets[g] = CS_ISO_LATIN1_SUPPLEMENTAL;    \
        } else                                            \
            SerialConnVT320::ProcessG##g##_CS96(p);       \
    } while (0)
//
void SerialConnVT420::ProcessG1_CS96(const std::string& p)
{
    DO_SET_CHARSET96(1);
}
void SerialConnVT420::ProcessG2_CS96(const std::string& p)
{
    DO_SET_CHARSET96(2);
}
void SerialConnVT420::ProcessG3_CS96(const std::string& p)
{
    DO_SET_CHARSET96(3);
}

void SerialConnVT420::ProcessDA(const std::string& p)
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
        Put("\033[?62,63,64;6;9;15;18c");
        break;
    default:
        SerialConnVT320::ProcessDA(p);
    }
}

void SerialConnVT420::ProcessSecondaryDA(const std::string& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 0:
        // CSI > 41; Pv; 1 c
        // Pv - firmware version.
        Put("\033[>41;20;1c");
        break;
    default:
        SerialConnVT320::ProcessSecondaryDA(p);
    }
}

void SerialConnVT420::ProcessTertiaryDA(const std::string& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 0:
        // DCS!|D...D ST
        Put("\033\x50!|0\033\x5c");
        break;
    default:
        break;
    }
}
// TODO: https://vt100.net/docs/vt420-uu/chapter9.html, 12 VT420 Reports
void SerialConnVT420::ProcessDECDSR(const std::string& p)
{
    int pn = atoi(p.data());
    switch (pn) {
    case 6:
        if (1) {
            std::string rsp = std::string("\033[?") + std::to_string(mVy) + ";" + std::to_string(mVx) + ";0R";
            Put(rsp);
        }
        break;
    default:
        SerialConnVT320::ProcessDECDSR(p);
    }
}
