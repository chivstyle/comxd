//
// (c) 2020 chiv
//
#include "SerialConnEcma48.h"
#include "Ecma48ControlSeq.h"
#include "ConnFactory.h"

using namespace Upp;

SerialConnEcma48::SerialConnEcma48(std::shared_ptr<SerialIo> io)
    : Superclass(io)
    , mSee(1)
    , mPageHome(0)
    , mLineHome(0)
    , mUseS8C(false)
{
    InstallFunctions();
    //
    AddEcma48ControlSeqs(this->mSeqsFactory);
}

SerialConnEcma48::~SerialConnEcma48()
{
}

void SerialConnEcma48::InstallFunctions()
{
    mFunctions[ECMA48_SOH] = [=](const std::string_view& p) { ProcessSOH(p); };
    mFunctions[ECMA48_STX] = [=](const std::string_view& p) { ProcessSTX(p); };
    mFunctions[ECMA48_ETX] = [=](const std::string_view& p) { ProcessETX(p); };
    mFunctions[ECMA48_EOT] = [=](const std::string_view& p) { ProcessEOT(p); };
    mFunctions[ECMA48_ENQ] = [=](const std::string_view& p) { ProcessENQ(p); };
    mFunctions[ECMA48_ACK] = [=](const std::string_view& p) { ProcessACK(p); };
    mFunctions[ECMA48_BEL] = [=](const std::string_view& p) { ProcessBEL(p); };
    mFunctions[ECMA48_BS] = [=](const std::string_view& p) { ProcessBS(p); };
    mFunctions[ECMA48_HT] = [=](const std::string_view& p) { ProcessHT(p); };
    mFunctions[ECMA48_LF] = [=](const std::string_view& p) { ProcessLF(p); };
    mFunctions[ECMA48_VT] = [=](const std::string_view& p) { ProcessVT(p); };
    mFunctions[ECMA48_FF] = [=](const std::string_view& p) { ProcessFF(p); };
    mFunctions[ECMA48_CR] = [=](const std::string_view& p) { ProcessCR(p); };
    mFunctions[ECMA48_SI] = [=](const std::string_view& p) { ProcessSI(p); };
    mFunctions[ECMA48_SO] = [=](const std::string_view& p) { ProcessSO(p); };
    mFunctions[ECMA48_DLE] = [=](const std::string_view& p) { ProcessDLE(p); };
    mFunctions[ECMA48_DC1] = [=](const std::string_view& p) { ProcessDC1(p); };
    mFunctions[ECMA48_DC2] = [=](const std::string_view& p) { ProcessDC2(p); };
    mFunctions[ECMA48_DC3] = [=](const std::string_view& p) { ProcessDC3(p); };
    mFunctions[ECMA48_DC4] = [=](const std::string_view& p) { ProcessDC4(p); };
    mFunctions[ECMA48_NAK] = [=](const std::string_view& p) { ProcessNAK(p); };
    mFunctions[ECMA48_SYN] = [=](const std::string_view& p) { ProcessSYN(p); };
    mFunctions[ECMA48_ETB] = [=](const std::string_view& p) { ProcessETB(p); };
    mFunctions[ECMA48_CAN] = [=](const std::string_view& p) { ProcessCAN(p); };
    mFunctions[ECMA48_EM] = [=](const std::string_view& p) { ProcessEM(p); };
    mFunctions[ECMA48_SUB] = [=](const std::string_view& p) { ProcessSUB(p); };
    mFunctions[ECMA48_FS] = [=](const std::string_view& p) { ProcessFS(p); };
    mFunctions[ECMA48_GS] = [=](const std::string_view& p) { ProcessGS(p); };
    mFunctions[ECMA48_RS] = [=](const std::string_view& p) { ProcessRS(p); };
    mFunctions[ECMA48_US] = [=](const std::string_view& p) { ProcessUS(p); };
    mFunctions[ECMA48_DEL] = [=](const std::string_view& p) { ProcessDEL(p); };
    mFunctions[ECMA48_APC] = [=](const std::string_view& p) { ProcessAPC(p); };
    mFunctions[ECMA48_BPH] = [=](const std::string_view& p) { ProcessBPH(p); };
    mFunctions[ECMA48_CBT] = [=](const std::string_view& p) { ProcessCBT(p); };
    mFunctions[ECMA48_CCH] = [=](const std::string_view& p) { ProcessCCH(p); };
    mFunctions[ECMA48_CHA] = [=](const std::string_view& p) { ProcessCHA(p); };
    mFunctions[ECMA48_CHT] = [=](const std::string_view& p) { ProcessCHT(p); };
    mFunctions[ECMA48_CMD] = [=](const std::string_view& p) { ProcessCMD(p); };
    mFunctions[ECMA48_CNL] = [=](const std::string_view& p) { ProcessCNL(p); };
    mFunctions[ECMA48_CPL] = [=](const std::string_view& p) { ProcessCPL(p); };
    mFunctions[ECMA48_CPR] = [=](const std::string_view& p) { ProcessCPR(p); };
    mFunctions[ECMA48_CTC] = [=](const std::string_view& p) { ProcessCTC(p); };
    mFunctions[ECMA48_CUB] = [=](const std::string_view& p) { ProcessCUB(p); };
    mFunctions[ECMA48_CUD] = [=](const std::string_view& p) { ProcessCUD(p); };
    mFunctions[ECMA48_CUF] = [=](const std::string_view& p) { ProcessCUF(p); };
    mFunctions[ECMA48_CUP] = [=](const std::string_view& p) { ProcessCUP(p); };
    mFunctions[ECMA48_CUF] = [=](const std::string_view& p) { ProcessCUF(p); };
    mFunctions[ECMA48_CUU] = [=](const std::string_view& p) { ProcessCUU(p); };
    mFunctions[ECMA48_CVT] = [=](const std::string_view& p) { ProcessCVT(p); };
    mFunctions[ECMA48_DA ] = [=](const std::string_view& p) { ProcessDA(p); };
    mFunctions[ECMA48_DAQ] = [=](const std::string_view& p) { ProcessDAQ(p); };
    mFunctions[ECMA48_DCH] = [=](const std::string_view& p) { ProcessDCH(p); };
    mFunctions[ECMA48_DCS] = [=](const std::string_view& p) { ProcessDCS(p); };
    mFunctions[ECMA48_DL ] = [=](const std::string_view& p) { ProcessDL(p); };
    mFunctions[ECMA48_DMI] = [=](const std::string_view& p) { ProcessDMI(p); };
    mFunctions[ECMA48_DSR] = [=](const std::string_view& p) { ProcessDSR(p); };
    mFunctions[ECMA48_DTA] = [=](const std::string_view& p) { ProcessDTA(p); };
    mFunctions[ECMA48_EA ] = [=](const std::string_view& p) { ProcessEA(p); };
    mFunctions[ECMA48_ECH] = [=](const std::string_view& p) { ProcessECH(p); };
    mFunctions[ECMA48_ED ] = [=](const std::string_view& p) { ProcessED(p); };
    mFunctions[ECMA48_EF ] = [=](const std::string_view& p) { ProcessEF(p); };
    mFunctions[ECMA48_EL ] = [=](const std::string_view& p) { ProcessEL(p); };
    mFunctions[ECMA48_EMI] = [=](const std::string_view& p) { ProcessEMI(p); };
    mFunctions[ECMA48_EPA] = [=](const std::string_view& p) { ProcessEPA(p); };
    mFunctions[ECMA48_ESA] = [=](const std::string_view& p) { ProcessESA(p); };
    mFunctions[ECMA48_FNK] = [=](const std::string_view& p) { ProcessFNK(p); };
    mFunctions[ECMA48_FNT] = [=](const std::string_view& p) { ProcessFNT(p); };
    mFunctions[ECMA48_GCC] = [=](const std::string_view& p) { ProcessGCC(p); };
    mFunctions[ECMA48_GSM] = [=](const std::string_view& p) { ProcessGSM(p); };
    mFunctions[ECMA48_GSS] = [=](const std::string_view& p) { ProcessGSS(p); };
    mFunctions[ECMA48_HPA] = [=](const std::string_view& p) { ProcessHPA(p); };
    mFunctions[ECMA48_HPB] = [=](const std::string_view& p) { ProcessHPB(p); };
    mFunctions[ECMA48_HPR] = [=](const std::string_view& p) { ProcessHPR(p); };
    mFunctions[ECMA48_HTJ] = [=](const std::string_view& p) { ProcessHTJ(p); };
    mFunctions[ECMA48_HTS] = [=](const std::string_view& p) { ProcessHTS(p); };
    mFunctions[ECMA48_HVP] = [=](const std::string_view& p) { ProcessHVP(p); };
    mFunctions[ECMA48_ICH] = [=](const std::string_view& p) { ProcessICH(p); };
    mFunctions[ECMA48_IDCS] = [=](const std::string_view& p) { ProcessIDCS(p); };
    mFunctions[ECMA48_IGS] = [=](const std::string_view& p) { ProcessIGS(p); };
    mFunctions[ECMA48_IL ] = [=](const std::string_view& p) { ProcessIL(p); };
    mFunctions[ECMA48_INT] = [=](const std::string_view& p) { ProcessINT(p); };
    mFunctions[ECMA48_JFY] = [=](const std::string_view& p) { ProcessJFY(p); };
    mFunctions[ECMA48_LS1R] = [=](const std::string_view& p) { ProcessLS1R(p); };
    mFunctions[ECMA48_LS2] = [=](const std::string_view& p) { ProcessLS2(p); };
    mFunctions[ECMA48_LS2R] = [=](const std::string_view& p) { ProcessLS2(p); };
    mFunctions[ECMA48_LS3] = [=](const std::string_view& p) { ProcessLS3(p); };
    mFunctions[ECMA48_LS3R] = [=](const std::string_view& p) { ProcessLS3(p); };
    mFunctions[ECMA48_MC ] = [=](const std::string_view& p) { ProcessMC(p); };
    mFunctions[ECMA48_MW ] = [=](const std::string_view& p) { ProcessMW(p); };
    mFunctions[ECMA48_NBH] = [=](const std::string_view& p) { ProcessNBH(p); };
    mFunctions[ECMA48_NEL] = [=](const std::string_view& p) { ProcessNEL(p); };
    mFunctions[ECMA48_NP ] = [=](const std::string_view& p) { ProcessNP(p); };
    mFunctions[ECMA48_OSC] = [=](const std::string_view& p) { ProcessOSC(p); };
    mFunctions[ECMA48_PEC] = [=](const std::string_view& p) { ProcessPEC(p); };
    mFunctions[ECMA48_PFS] = [=](const std::string_view& p) { ProcessPFS(p); };
    mFunctions[ECMA48_PLD] = [=](const std::string_view& p) { ProcessPLD(p); };
    mFunctions[ECMA48_PLU] = [=](const std::string_view& p) { ProcessPLU(p); };
    mFunctions[ECMA48_PM ] = [=](const std::string_view& p) { ProcessPM(p); };
    mFunctions[ECMA48_PP ] = [=](const std::string_view& p) { ProcessPP(p); };
    mFunctions[ECMA48_PPA] = [=](const std::string_view& p) { ProcessPPA(p); };
    mFunctions[ECMA48_PPB] = [=](const std::string_view& p) { ProcessPPB(p); };
    mFunctions[ECMA48_PPR] = [=](const std::string_view& p) { ProcessPPR(p); };
    mFunctions[ECMA48_PTX] = [=](const std::string_view& p) { ProcessPTX(p); };
    mFunctions[ECMA48_PU1] = [=](const std::string_view& p) { ProcessPU1(p); };
    mFunctions[ECMA48_PU2] = [=](const std::string_view& p) { ProcessPU2(p); };
    mFunctions[ECMA48_QUAD] = [=](const std::string_view& p) { ProcessQUAD(p); };
    mFunctions[ECMA48_REP] = [=](const std::string_view& p) { ProcessREP(p); };
    mFunctions[ECMA48_RI ] = [=](const std::string_view& p) { ProcessRI(p); };
    mFunctions[ECMA48_RIS] = [=](const std::string_view& p) { ProcessRIS(p); };
    mFunctions[ECMA48_RM ] = [=](const std::string_view& p) { ProcessRM(p); };
    mFunctions[ECMA48_SACS] = [=](const std::string_view& p) { ProcessSACS(p); };
    mFunctions[ECMA48_SAPV] = [=](const std::string_view& p) { ProcessSAPV(p); };
    mFunctions[ECMA48_SCI] = [=](const std::string_view& p) { ProcessSCI(p); };
    mFunctions[ECMA48_SCO] = [=](const std::string_view& p) { ProcessSCO(p); };
    mFunctions[ECMA48_SCP] = [=](const std::string_view& p) { ProcessSCP(p); };
    mFunctions[ECMA48_SCS] = [=](const std::string_view& p) { ProcessSCS(p); };
    mFunctions[ECMA48_SD ] = [=](const std::string_view& p) { ProcessSD(p); };
    mFunctions[ECMA48_SDS] = [=](const std::string_view& p) { ProcessSDS(p); };
    mFunctions[ECMA48_SEE] = [=](const std::string_view& p) { ProcessSEE(p); };
    mFunctions[ECMA48_SEF] = [=](const std::string_view& p) { ProcessSEF(p); };
    mFunctions[ECMA48_SGR] = [=](const std::string_view& p) { ProcessSGR(p); };
    mFunctions[ECMA48_SHS] = [=](const std::string_view& p) { ProcessSHS(p); };
    mFunctions[ECMA48_SIMD] = [=](const std::string_view& p) { ProcessSIMD(p); };
    mFunctions[ECMA48_SL ] = [=](const std::string_view& p) { ProcessSL(p); };
    mFunctions[ECMA48_SLH] = [=](const std::string_view& p) { ProcessSLH(p); };
    mFunctions[ECMA48_SLL] = [=](const std::string_view& p) { ProcessSLL(p); };
    mFunctions[ECMA48_SLS] = [=](const std::string_view& p) { ProcessSLS(p); };
    mFunctions[ECMA48_SM ] = [=](const std::string_view& p) { ProcessSM(p); };
    mFunctions[ECMA48_SOS] = [=](const std::string_view& p) { ProcessSOS(p); };
    mFunctions[ECMA48_SPA] = [=](const std::string_view& p) { ProcessSPA(p); };
    mFunctions[ECMA48_SPD] = [=](const std::string_view& p) { ProcessSPD(p); };
    mFunctions[ECMA48_SPH] = [=](const std::string_view& p) { ProcessSPH(p); };
    mFunctions[ECMA48_SPI] = [=](const std::string_view& p) { ProcessSPI(p); };
    mFunctions[ECMA48_SPL] = [=](const std::string_view& p) { ProcessSPL(p); };
    mFunctions[ECMA48_SPQR] = [=](const std::string_view& p) { ProcessSPQR(p); };
    mFunctions[ECMA48_SR ] = [=](const std::string_view& p) { ProcessSR(p); };
    mFunctions[ECMA48_SRCS] = [=](const std::string_view& p) { ProcessSRCS(p); };
    mFunctions[ECMA48_SRS] = [=](const std::string_view& p) { ProcessSRS(p); };
    mFunctions[ECMA48_SSA] = [=](const std::string_view& p) { ProcessSSA(p); };
    mFunctions[ECMA48_SSU] = [=](const std::string_view& p) { ProcessSSU(p); };
    mFunctions[ECMA48_SSW] = [=](const std::string_view& p) { ProcessSSW(p); };
    mFunctions[ECMA48_SS2] = [=](const std::string_view& p) { ProcessSS2(p); };
    mFunctions[ECMA48_SS3] = [=](const std::string_view& p) { ProcessSS3(p); };
    mFunctions[ECMA48_ST ] = [=](const std::string_view& p) { ProcessST(p); };
    mFunctions[ECMA48_STAB] = [=](const std::string_view& p) { ProcessSTAB(p); };
    mFunctions[ECMA48_STS] = [=](const std::string_view& p) { ProcessSTS(p); };
    mFunctions[ECMA48_STX] = [=](const std::string_view& p) { ProcessSTX(p); };
    mFunctions[ECMA48_SU ] = [=](const std::string_view& p) { ProcessSU(p); };
    mFunctions[ECMA48_SVS] = [=](const std::string_view& p) { ProcessSVS(p); };
    mFunctions[ECMA48_TAC] = [=](const std::string_view& p) { ProcessTAC(p); };
    mFunctions[ECMA48_TALE] = [=](const std::string_view& p) { ProcessTALE(p); };
    mFunctions[ECMA48_TATE] = [=](const std::string_view& p) { ProcessTATE(p); };
    mFunctions[ECMA48_TBC] = [=](const std::string_view& p) { ProcessTBC(p); };
    mFunctions[ECMA48_TCC] = [=](const std::string_view& p) { ProcessTCC(p); };
    mFunctions[ECMA48_TSR] = [=](const std::string_view& p) { ProcessTSR(p); };
    mFunctions[ECMA48_TSS] = [=](const std::string_view& p) { ProcessTSS(p); };
    mFunctions[ECMA48_VPA] = [=](const std::string_view& p) { ProcessVPA(p); };
    mFunctions[ECMA48_VPB] = [=](const std::string_view& p) { ProcessVPB(p); };
    mFunctions[ECMA48_VPR] = [=](const std::string_view& p) { ProcessVPR(p); };
    mFunctions[ECMA48_VTS] = [=](const std::string_view& p) { ProcessVTS(p); };
}
//
void SerialConnEcma48::Fill(int X0, int Y0, int X1, int Y1, const VTChar& c)
{
    if (Y0 > Y1) { // top left
        std::swap(Y0, Y1);
        std::swap(X0, X1);
    } else if (Y0 == Y1) {
        if (X0 > X1) {
            std::swap(X0, X1);
        }
    }
    if (X0 == X1 && Y0 == Y1) return;
    //
    int nlines = Y1 - Y0;
    if (nlines) {
        // head
        for (int i = X0; i < (int)mLines[Y0].size(); ++i) {
            mLines[Y0][i] = c;
        }
        // body
        Size csz = GetConsoleSize();
        for (int i = 0; i < nlines-1; ++i) {
            mLines[Y0+i] = VTLine(csz.cx, c).SetHeight(mFontH);
        }
        // tail
        for (int i = 0; i < X1; ++i) {
            mLines[Y1][i] = c;
        }
    } else {
        for (int i = X0; i < X1; ++i) {
            mLines[Y0][i] = c;
        }
    }
}
// C0, Start of heading, Not used
void SerialConnEcma48::ProcessSOH(const std::string_view&)
{
}
// C0, Start of text, not used
void SerialConnEcma48::ProcessSTX(const std::string_view&)
{
}
// C0, Start of tetx, not used
void SerialConnEcma48::ProcessETX(const std::string_view&)
{
}
// C0, End of transmission, not used
void SerialConnEcma48::ProcessEOT(const std::string_view&)
{
}
// C0, Enquiry
void SerialConnEcma48::ProcessENQ(const std::string_view&)
{
}
// C0, Acknowledge
void SerialConnEcma48::ProcessACK(const std::string_view&)
{
}
// C0, Bell
void SerialConnEcma48::ProcessBEL(const std::string_view&)
{
	Upp::BeepExclamation();
}
// C0, Backspace
void SerialConnEcma48::ProcessBS(const std::string_view&)
{
	int px = mPx - mFontW;
	if (px < 0) {
		Size csz = GetConsoleSize();
        int vy = mVy - 1;
        // this line and previous line belong to a single line, because the last
        // line was not broken by a LineFeed or Vertical tabulation.
        if (vy >= 0) {
            VTLine& vline = mLines[vy];
            if (vline.HasSuccessiveLines()) {
	            mPx = this->VirtualToLogic(vline, csz.cx, false) - mFontW;
	            mVy = vy;
            }
        }
	} else {
		mPx = px;
	}
}
// C0, horizon tab
void SerialConnEcma48::ProcessHT(const std::string_view&)
{
    int tabsz = mFontW*mTabWidth;
    mPx += tabsz - (mPx % tabsz);
}
// C0, Line feed
void SerialConnEcma48::ProcessLF(const std::string_view&)
{
    mVy += 1;
    if (mModes.LMN == Ecma48Modes::LMN_LineFeed) {
        mVx = mLineHome;
    }
}
// C0, Vertical tab
void SerialConnEcma48::ProcessVT(const std::string_view&)
{
	ProcessLF("");
}
// C0, Form feed
void SerialConnEcma48::ProcessFF(const std::string_view&)
{
	ProcessLF("");
}
// C0, Carrier
void SerialConnEcma48::ProcessCR(const std::string_view&)
{
    mVx = 0;
}
// C0, Shift in
void SerialConnEcma48::ProcessSI(const std::string_view&)
{
}
// C0, Shift out
void SerialConnEcma48::ProcessSO(const std::string_view&)
{
}
// C0, Data link escape
void SerialConnEcma48::ProcessDLE(const std::string_view&)
{
}
// C0, Device control, 1-4
void SerialConnEcma48::ProcessDC1(const std::string_view&)
{
}
void SerialConnEcma48::ProcessDC2(const std::string_view&)
{
}
void SerialConnEcma48::ProcessDC3(const std::string_view&)
{
}
void SerialConnEcma48::ProcessDC4(const std::string_view&)
{
}
// C0, Negative acknowledge
void SerialConnEcma48::ProcessNAK(const std::string_view&)
{
}
// C0, Synchronous idle
void SerialConnEcma48::ProcessSYN(const std::string_view&)
{
}
// C0, End of block
void SerialConnEcma48::ProcessETB(const std::string_view&)
{
}
// C0, Cancel
void SerialConnEcma48::ProcessCAN(const std::string_view&)
{
}
// C0, End of medium
void SerialConnEcma48::ProcessEM(const std::string_view&)
{
}
// C0, Substitute
void SerialConnEcma48::ProcessSUB(const std::string_view&)
{
}
// C0, File separator
void SerialConnEcma48::ProcessFS(const std::string_view&)
{
}
// C0, Group separator
void SerialConnEcma48::ProcessGS(const std::string_view&)
{
}
// C0, Record separator
void SerialConnEcma48::ProcessRS(const std::string_view&)
{
}
// C0, Unit separator
void SerialConnEcma48::ProcessUS(const std::string_view&)
{
}
// C0, Delete
void SerialConnEcma48::ProcessDEL(const std::string_view&)
{
}
// Application program command
void SerialConnEcma48::ProcessAPC(const std::string_view& p)
{
}
// Bread permitted here
void SerialConnEcma48::ProcessBPH(const std::string_view& p)
{
}
// Cursor backward tab
void SerialConnEcma48::ProcessCBT(const std::string_view& p)
{
    int tabsz = mFontW*mTabWidth;
    int pn = atoi(p.data());
    if (pn <= 0) pn = 1;
    //
    int rn = mPx % tabsz;
    if (rn)
        pn -= 1;
    mPx -= rn + tabsz*pn;
}
// Cancel character
void SerialConnEcma48::ProcessCCH(const std::string_view& p)
{
    VTLine& vline = mLines[mVy];
    if (mVx > 0) {
        vline[mVx-1] = mBlankChar;
        mVx--;
    }
}
// Cursor character absolute
void SerialConnEcma48::ProcessCHA(const std::string_view& p)
{
    int pn = atoi(p.data());
    if (pn <= 0) pn = 1;
    mPx = mFontW*(pn-1);
}
// Cursor forward tab
void SerialConnEcma48::ProcessCHT(const std::string_view& p)
{
    int tabsz = mFontW*mTabWidth;
    int pn = atoi(p.data());
    if (pn <= 0) pn = 1;
    //
    int rn = tabsz - (mPx % tabsz);
    if (rn)
        pn -= 1;
    mPx += rn + tabsz*pn;
}
// Coding method delimiter
void SerialConnEcma48::ProcessCMD(const std::string_view& p)
{
}
// Cursor next line
void SerialConnEcma48::ProcessCNL(const std::string_view& p)
{
    int pn = atoi(p.data());
    if (pn <= 0) pn = 1;
    mVy += pn;
    mVx = 0;
}
// Cursor preceding line
void SerialConnEcma48::ProcessCPL(const std::string_view& p)
{
    int pn = atoi(p.data());
    if (pn <= 0) pn = 1;
    mVy -= pn;
}
// CPR is used to report cursor position
void SerialConnEcma48::ProcessCPR(const std::string_view& p)
{
}
// TODO: it's not correct, maybe. I have no idea how to implement this
void SerialConnEcma48::ProcessCTC(const std::string_view& p)
{
    SplitString(p.data(), ';', [=](const char* token) {
        int ps = atoi(token);
        switch (ps) {
        case 0:
            mLines[mVy][mVx] ='\t';
            mVx++;
            break;
        case 1:
            mLines[mVy][mVx] = '\v';
            mVy += 1;
            break;
        case 2: if (1) {
            VTLine& vline = mLines[mVy];
            if (vline[mVx] == '\t') {
                vline.erase(vline.begin() + mVx);
                vline.push_back(mBlankChar);
            }
        } break;
        case 3: if (1) {
            Size csz = GetConsoleSize();
            VTLine& vline = mLines[mVy];
            if (vline[mVx] == '\v') {
                vline[mVx] = ' ';
                if (mVy < (int)mLines.size() - 1) {
                    VTLine& vline_next = mLines[mVy+1];
                    vline.insert(vline.begin() + mVx,
                        vline_next.begin(), vline_next.end());
                    mLines.erase(mLines.begin() + mVy);
                    mLines.push_back(VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
                }
            }
        } break;
        case 4: if (1) {
            int ntabs = 0;
            VTLine& vline = mLines[mVy];
            for (auto it = vline.begin(); it != vline.end(); ) {
                if (*it == '\t') {
                    it = vline.erase(it);
                    ntabs++;
                } else ++it;
            }
            if (ntabs)
                vline.insert(vline.end(), ntabs, mBlankChar);
        } break;
        case 5: if (1) {
            ProcessCTC("6;4");
        } break;
        case 6: if (1) {
            Size csz = GetConsoleSize();
            int ntabs = 0;
            for (auto it = mLines.begin(); it != mLines.end(); ) {
                if (*it->rbegin() == '\v') {
                    if (it + 1 != mLines.end()) {
                        VTLine& vline = *it;
                        vline.pop_back();
                        VTLine& vline_next = *(it+1);
                        vline.insert(vline.end(), vline_next.begin(), vline_next.end());
                        ntabs++;
                        it = mLines.erase(it+1);
                    }
                } else ++it;
            }
            mLines.insert(mLines.end(), ntabs, VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
        } break;
        }
    });
}
void SerialConnEcma48::ProcessCUB(const std::string_view& p)
{
    int pn = atoi(p.data());
    if (pn <= 0) pn = 1;
    mPx -= mFontW*pn;
}
void SerialConnEcma48::ProcessCUD(const std::string_view& p)
{
    Size csz = GetConsoleSize();
    int pn = atoi(p.data());
    if (pn <= 0) pn = 1;
    mVy += pn;
}
void SerialConnEcma48::ProcessCUF(const std::string_view& p)
{
    int pn = atoi(p.data());
    if (pn <= 0) pn = 1;
    mPx += mFontW*pn;
}
void SerialConnEcma48::ProcessCUP(const std::string_view& p)
{
    int idx = 0, pn[2] = {1, 1};
    SplitString(p.data(), ';', [=, &idx, &pn](const char* token) {
        if (idx < 2)
            pn[idx++] = atoi(token);
    });
    if (pn[0] <= 0) pn[0] = 1;
    if (pn[1] <= 0) pn[1] = 1;
    mPx = mFontW*(pn[1]-1);
    mVy = pn[0]-1;
}
void SerialConnEcma48::ProcessCUU(const std::string_view& p)
{
    int pn = atoi(p.data());
    if (pn <= 0) pn = 1;
    mVy -= pn;
}
void SerialConnEcma48::ProcessCVT(const std::string_view& p)
{
    int pn = atoi(p.data());
    mVy += pn;
}
void SerialConnEcma48::ProcessDA(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessDAQ(const std::string_view& p)
{
	int ps = atoi(p.data());
	if (ps >= 0 && ps < 12) {
		mDaq[ps].From = Point(mVx, mVy);
	}
}
void SerialConnEcma48::ProcessDCH(const std::string_view& p)
{
    int pn = atoi(p.data());
    if (pn <= 0) pn = 1;
    // TODO: process modes
    VTLine& vline = mLines[mVy];
    if (mVx + pn >= (int)vline.size()) {
        pn = (int)vline.size() - mVx;
    }
    vline.erase(vline.begin() + mVx, vline.begin() + mVx+pn);
    vline.insert(vline.end(), pn, mBlankChar);
}
// Device control string
void SerialConnEcma48::ProcessDCS(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessDL(const std::string_view& p)
{
    int pn = atoi(p.data());
    if (pn <= 0) pn = 1;
    // scrolling region
    int top = mScrollingRegion.Top;
    int bot = mScrollingRegion.Bottom;
    if (bot < 0) bot = (int)mLines.size()-1;
    if (mVy < top || mVy > bot) return; // invalid
    int ln = bot-mVy+1;
    if (pn > ln) pn = ln;
    auto it_end = mLines.begin() + bot + 1;
    Size csz = GetConsoleSize();
    mLines.insert(it_end, pn, VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
    mLines.erase(mLines.begin()+mVy, mLines.begin()+mVy+pn);
}
// disable manual input
void SerialConnEcma48::ProcessDMI(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessDSR(const std::string_view& p)
{
	int ps = atoi(p.data());
    switch (ps) {
    case 5:
        Put("\x1b[0\x6e"); // It's OK, no malfunction detected
        break;
    case 6: if (1) {
        std::string cpr = std::string("\x1b[") + std::to_string(mPy/mFontH + 1)
            + ";" + std::to_string(mPx/mFontW + 1) + "\x52";
        Put(cpr);
    } break;
    }
}
void SerialConnEcma48::ProcessDTA(const std::string_view& p)
{
	Size csz = GetConsoleSize();
    int idx = 0, pn[2] = {1, 1};
    SplitString(p.data(), ';', [=, &idx, &pn](const char* token) {
        if (idx < 2)
            pn[idx++] = atoi(token);
    });
    if (pn[0] <= 0) pn[0] = 1; //
    if (pn[1] <= 0) pn[1] = 1; //
    if (pn[0] < pn[1] && pn[1] <= csz.cy) {
        mScrollingRegion.Top = pn[0]-1;
        mScrollingRegion.Bottom = pn[1]-1;
        // To the home position
        mVx = 0;
        mVy = pn[0]-1;
    }
}
// Erase in area, we ignore it.
void SerialConnEcma48::ProcessEA(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessECH(const std::string_view& p)
{
	int pn = atoi(p.data());
	if (pn <= 0) pn = 1;
	VTLine& vline = mLines[mVy];
	for (int i = mVx; i < mVx+pn && i < (int)vline.size(); ++i) {
		vline[i] = mBlankChar; // put to erased state
	}
}
void SerialConnEcma48::ProcessED(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 0: if (1) {
        VTLine& vline = mLines[mVy];
        for (int i = mVx; i < (int)vline.size(); ++i) {
            vline[i] = mBlankChar;
        }
        for (int j = mVy+1; j < (int)mLines.size(); ++j) {
            VTLine& vline = mLines[j];
            for (int i = 0; i < (int)vline.size(); ++i) {
                vline[i] = mBlankChar;
            }
        }
    } break;
    case 1: if (1) {
        VTLine& vline = mLines[mVy];
        for (int i = 0; i < (int)vline.size() && i <= mVx; ++i) {
            vline[i] = mBlankChar;
        }
        for (int j = mVy-1; j >= 0; --j) {
            VTLine& vline = mLines[j];
            for (int i = 0; i < (int)vline.size(); ++i) {
                vline[i] = mBlankChar;
            }
        }
    } break;
    case 2: if (1) {
        for (int j = 0; j < (int)mLines.size(); ++j) {
            VTLine& vline = mLines[j];
            for (int i = 0; i < (int)vline.size(); ++i) {
                vline[i] = mBlankChar;
            }
        }
    } break;
    }
}
void SerialConnEcma48::ProcessEF(const std::string_view& p)
{
	ProcessED(p);
}
void SerialConnEcma48::ProcessEL(const std::string_view& p)
{
    int ps = atoi(p.data());
    switch (ps) {
    case 0: if (1) {
        VTLine& vline = mLines[mVy];
        for (int i = mVx; i < (int)vline.size(); ++i) {
            vline[i] = mBlankChar;
        }
    } break;
    case 1: if (1) {
        VTLine& vline = mLines[mVy];
        for (int i = 0; i < (int)vline.size() && i <= mVx; ++i) {
            vline[i] = mBlankChar;
        }
    } break;
    case 2: if (1) {
        Size csz = GetConsoleSize();
        mLines[mVy] = VTLine(csz.cx, mBlankChar).SetHeight(mFontH);
        break;
    } break;
    }
}
// Enable manual input
void SerialConnEcma48::ProcessEMI(const std::string_view& p)
{
}
// End of guarded area, we ignore it
void SerialConnEcma48::ProcessEPA(const std::string_view& p)
{
}
// End of selected area, we ignore it
void SerialConnEcma48::ProcessESA(const std::string_view& p)
{
}
// Function key
void SerialConnEcma48::ProcessFNK(const std::string_view& p)
{
}
// Font selection, we ignore it
void SerialConnEcma48::ProcessFNT(const std::string_view& p)
{
}
// Graphics character combination
void SerialConnEcma48::ProcessGCC(const std::string_view& p)
{
}
// Graphics size modification
void SerialConnEcma48::ProcessGSM(const std::string_view& p)
{
}
// Graphics size selection
void SerialConnEcma48::ProcessGSS(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessHPA(const std::string_view& p)
{
	// data component
	int pn = atoi(p.data());
	if (pn <= 0) pn = 1;
	mVx = pn - 1; // To absolute position
}
void SerialConnEcma48::ProcessHPB(const std::string_view& p)
{
	int pn = atoi(p.data());
	if (pn <= 0) pn = 1;
	mVx -= pn;
}
void SerialConnEcma48::ProcessHPR(const std::string_view& p)
{
	// data component
	int pn = atoi(p.data());
	if (pn <= 0) pn = 1;
	mVx += pn;
}
// Character tabulation with justification
void SerialConnEcma48::ProcessHTJ(const std::string_view& p)
{
}
// Character tabulation set
void SerialConnEcma48::ProcessHTS(const std::string_view& p)
{
	mLines[mVy][mVx++] = '\t';
}
void SerialConnEcma48::ProcessHVP(const std::string_view& p)
{
    ProcessCUP(p);
}
void SerialConnEcma48::ProcessICH(const std::string_view& p)
{
	int pn = atoi(p.data());
	if (pn <= 0) pn = 1;
	VTLine& vline = mLines[mVy];
	vline.insert(vline.begin() + mVx, pn, mBlankChar);
}
// Identify device control string
void SerialConnEcma48::ProcessIDCS(const std::string_view& p)
{
}
// Identify graphic subrepertoire
void SerialConnEcma48::ProcessIGS(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessIL(const std::string_view& p)
{
    int pn = atoi(p.data());
    if (pn <= 0) pn = 1;
    // scrolling region
    int top = mScrollingRegion.Top;
    int bot = mScrollingRegion.Bottom;
    if (bot < 0) bot = (int)mLines.size()-1;
    if (mVy < top || mVy > bot) return; // Invalid
    int ln = bot-mVy+1;
    if (pn > ln) pn = ln;
    auto it_bot = mLines.begin()+bot;
    mLines.erase(it_bot - pn+1, it_bot+1);
    Size csz = GetConsoleSize();
    mLines.insert(mLines.begin()+mVy, pn, VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
}
// Interrupt
void SerialConnEcma48::ProcessINT(const std::string_view& p)
{
}
// Justify
void SerialConnEcma48::ProcessJFY(const std::string_view& p)
{
}
//
void SerialConnEcma48::ProcessLS1R(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessLS2(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessLS2R(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessLS3(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessLS3R(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessMC(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessMW(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessNBH(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessNEL(const std::string_view& p)
{
    int bot = mScrollingRegion.Bottom;
    if (bot < 0) bot = (int)mLines.size()-1;
    if (mVy < bot) {
        mVy++;
    } else { // scroll up
        int top = mScrollingRegion.Top;
        if (bot < 0) bot = (int)mLines.size()-1;
        Size csz = GetConsoleSize();
        auto it_end = mLines.begin() + bot + 1;
        // insert new line
        mLines.insert(it_end, VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
        // remove the top line
        mLines.erase(mLines.begin() + top);
    }
    mVx = 0;
}
// Next page
void SerialConnEcma48::ProcessNP(const std::string_view& p)
{
}
// operating system command
void SerialConnEcma48::ProcessOSC(const std::string_view& p)
{
}
// presentation expand or contract
void SerialConnEcma48::ProcessPEC(const std::string_view& p)
{
}
// page format selection
void SerialConnEcma48::ProcessPFS(const std::string_view& p)
{
}
// partial line forward
void SerialConnEcma48::ProcessPLD(const std::string_view& p)
{
}
// partial line backward
void SerialConnEcma48::ProcessPLU(const std::string_view& p)
{
}
// privacy message
void SerialConnEcma48::ProcessPM(const std::string_view& p)
{
}
// preceding page
void SerialConnEcma48::ProcessPP(const std::string_view& p)
{
}
// page position absolute
void SerialConnEcma48::ProcessPPA(const std::string_view& p)
{
}
// page position backward
void SerialConnEcma48::ProcessPPB(const std::string_view& p)
{
}
// page position forward
void SerialConnEcma48::ProcessPPR(const std::string_view& p)
{
}
// parallel texts
void SerialConnEcma48::ProcessPTX(const std::string_view& p)
{
}
// private use one
void SerialConnEcma48::ProcessPU1(const std::string_view& p)
{
}
// private use two
void SerialConnEcma48::ProcessPU2(const std::string_view& p)
{
}
// quad
void SerialConnEcma48::ProcessQUAD(const std::string_view& p)
{
}
// repeat last char
void SerialConnEcma48::ProcessREP(const std::string_view& p)
{
	int pn = atoi(p.data());
	if (pn <= 0) pn = 1;
	VTLine& vline = mLines[mVy];
	const VTChar& ch = mVx > 0 ? vline[mVx - 1] : mBlankChar;
	int cn = 0;
	for (int i = mVx; i < (int)vline.size() && cn < pn; ++i) {
		vline[i] = ch;
		cn++;
	}
	cn = pn - cn;
	while (cn--)
		vline.push_back(ch);
	mVx += pn;
}
void SerialConnEcma48::ProcessRI(const std::string_view& p)
{
    int top = mScrollingRegion.Top;
    if (mVy > top) {
        mVy--;
    } else { // scroll down
        int bot = mScrollingRegion.Bottom;
        if (bot < 0) bot = (int)mLines.size()-1;
        mLines.erase(mLines.begin()+bot); // remove the bottom line
        // insert new line to top
        Size csz = GetConsoleSize();
        mLines.insert(mLines.begin() + top, VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
    }
}
void SerialConnEcma48::ProcessRIS(const std::string_view& p)
{
	this->Clear();
}
void SerialConnEcma48::ProcessRM(const std::string_view& p)
{
    SplitString(p.data(), ';', [=](const char* token) {
        int ps = atoi(token);
        switch (ps) {
        case 1:  mModes.GATM = 0; break;
        case 2:  mModes.KAM  = 0; break;
        case 3:  mModes.CRM  = 0; break;
        case 4:  mModes.IRM  = 0; break;
        case 5:  mModes.SRTM = 0; break;
        case 6:  mModes.ERM  = 0; break;
        case 7:  mModes.BDSM = 0; break;
        case 8:  mModes.BDSM = 0; break;
        case 9:  mModes.DCSM = 0; break;
        case 10: mModes.HEM  = 0; break;
        case 11: mModes.PUM  = 0; break;
        case 12: mModes.SRM  = 0; break;
        case 13: mModes.FEAM = 0; break;
        case 14: mModes.FETM = 0; break;
        case 15: mModes.MATM = 0; break;
        case 16: mModes.TTM  = 0; break;
        case 17: mModes.SATM = 0; break;
        case 18: mModes.TSM  = 0; break;
        case 19: mModes.EBM  = 0; break;
        case 20: mModes.LMN  = 0; break;
        case 21: mModes.GRCM = 0; break;
        case 22: mModes.ZDM  = 0; break;
        }
    });
}
// set additional character separation
void SerialConnEcma48::ProcessSACS(const std::string_view& p)
{
}
// select alternative presentation variants
void SerialConnEcma48::ProcessSAPV(const std::string_view& p)
{
}
// single character introducer
void SerialConnEcma48::ProcessSCI(const std::string_view& p)
{
}
// select character orientation
void SerialConnEcma48::ProcessSCO(const std::string_view& p)
{
}
// select character path
void SerialConnEcma48::ProcessSCP(const std::string_view& p)
{
}
// set character spacing
void SerialConnEcma48::ProcessSCS(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessSD(const std::string_view& p)
{
	int pn = atoi(p.data());
	if (pn <= 0) pn = 1;
	Size csz = GetConsoleSize();
	int top = mScrollingRegion.Top;
	int bot = mScrollingRegion.Bottom;
	mLines.insert(mLines.begin() + top, pn, VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
	auto it_bot = mLines.begin() + bot + pn;
	mLines.erase(it_bot - pn + 1, it_bot + 1);
}
// start directed string
void SerialConnEcma48::ProcessSDS(const std::string_view& p)
{
}
// selected editing extent
void SerialConnEcma48::ProcessSEE(const std::string_view& p)
{
	this->mSee = atoi(p.data());
}
// select eject and feed
void SerialConnEcma48::ProcessSEF(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessSGR(const std::string_view& p)
{
    SplitString(p.data(), ';', [=](const char* token) {
        int ps = atoi(token);
        switch (ps) {
        case 0:
            this->SetDefaultStyle();
            break;
        case 1:
            mStyle.FontStyle |= VTStyle::eBold;
            break;
        case 2: break;
        case 3:
            mStyle.FontStyle |= VTStyle::eItalic;
            break;
        case 4:
            mStyle.FontStyle |= VTStyle::eUnderline;
            break;
        case 5:
        case 6:
            mStyle.FontStyle |= VTStyle::eBlink;
            break;
        case 7:
            std::swap(mStyle.FgColorId, mStyle.BgColorId);
            break;
        case 8:
            mStyle.FontStyle &= ~VTStyle::eVisible;
            break;
        case 9:
            mStyle.FontStyle |= VTStyle::eStrikeout;
            break;
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
            break;
        case 21:
            mStyle.FontStyle |= VTStyle::eUnderline;
            break;
        case 22:
            mStyle.FgColorId = VTColorTable::kColorId_Texts;
            mStyle.BgColorId = VTColorTable::kColorId_Paper;
            mStyle.FontStyle &= ~VTStyle::eBold;
            break;
        case 23:
            mStyle.FontStyle &= ~VTStyle::eItalic;
            break;
        case 24:
            mStyle.FontStyle &= ~VTStyle::eUnderline;
            break;
        case 25:
            mStyle.FontStyle &= ~VTStyle::eBlink;
            break;
        case 26: break;
        case 27:
            std::swap(mStyle.FgColorId, mStyle.BgColorId);
            break;
        case 28:
            mStyle.FontStyle |= VTStyle::eVisible;
            break;
        case 29:
            mStyle.FontStyle &= ~VTStyle::eStrikeout;
            break;
        case 30:
            mStyle.FgColorId = VTColorTable::kColorId_Black;
            break;
        case 31:
            mStyle.FgColorId = VTColorTable::kColorId_Red;
            break;
        case 32:
            mStyle.FgColorId = VTColorTable::kColorId_Green;
            break;
        case 33:
            mStyle.FgColorId = VTColorTable::kColorId_Yellow;
            break;
        case 34:
            mStyle.FgColorId = VTColorTable::kColorId_Blue;
            break;
        case 35:
            mStyle.FgColorId = VTColorTable::kColorId_Magenta;
            break;
        case 36:
            mStyle.FgColorId = VTColorTable::kColorId_Cyan;
            break;
        case 37:
            mStyle.FgColorId = VTColorTable::kColorId_White;
            break;
        case 38: break;
        case 39:
            mStyle.FgColorId = VTColorTable::kColorId_Texts;
            break;
        case 40:
            mStyle.BgColorId = VTColorTable::kColorId_Paper;
            break;
        case 41:
            mStyle.BgColorId = VTColorTable::kColorId_Red;
            break;
        case 42:
            mStyle.BgColorId = VTColorTable::kColorId_Green;
            break;
        case 43:
            mStyle.BgColorId = VTColorTable::kColorId_Yellow;
            break;
        case 44:
            mStyle.BgColorId = VTColorTable::kColorId_Blue;
            break;
        case 45:
            mStyle.BgColorId = VTColorTable::kColorId_Magenta;
            break;
        case 46:
            mStyle.BgColorId = VTColorTable::kColorId_Cyan;
            break;
        case 47:
            mStyle.BgColorId = VTColorTable::kColorId_White;
            break;
        case 48: break;
        case 49:
            mStyle.BgColorId = VTColorTable::kColorId_Paper;
            break;
        case 50: break;
        // dose not support others.
        default: break;
        }
    });
}
// select character spacing
void SerialConnEcma48::ProcessSHS(const std::string_view& p)
{
}
// select implicit movement direction
void SerialConnEcma48::ProcessSIMD(const std::string_view& p)
{
}
// scroll left
void SerialConnEcma48::ProcessSL(const std::string_view& p)
{
	int pn = atoi(p.data());
	if (pn <= 0) pn = 1;
	for (size_t k = 0; k < mLines.size(); ++k) {
		VTLine& vline = mLines[k];
		vline.erase(vline.begin(), vline.begin()+pn);
		vline.insert(vline.end(), pn, mBlankChar);
	}
}
// set line home
void SerialConnEcma48::ProcessSLH(const std::string_view& p)
{
	int pn = atoi(p.data());
	if (pn <= 0) pn = 1;
	mLineHome = pn -1;
}
// set line limit, ignore it, we use unlimited line size
void SerialConnEcma48::ProcessSLL(const std::string_view& p)
{
}
// set line spacing
void SerialConnEcma48::ProcessSLS(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessSM(const std::string_view& p)
{
    SplitString(p.data(), ';', [=](const char* token) {
        int ps = atoi(token);
        switch (ps) {
        case 1:  mModes.GATM = 1; break;
        case 2:  mModes.KAM  = 1; break;
        case 3:  mModes.CRM  = 1; break;
        case 4:  mModes.IRM  = 1; break;
        case 5:  mModes.SRTM = 1; break;
        case 6:  mModes.ERM  = 1; break;
        case 7:  mModes.BDSM = 1; break;
        case 8:  mModes.BDSM = 1; break;
        case 9:  mModes.DCSM = 1; break;
        case 10: mModes.HEM  = 1; break;
        case 11: mModes.PUM  = 1; break;
        case 12: mModes.SRM  = 1; break;
        case 13: mModes.FEAM = 1; break;
        case 14: mModes.FETM = 1; break;
        case 15: mModes.MATM = 1; break;
        case 16: mModes.TTM  = 1; break;
        case 17: mModes.SATM = 1; break;
        case 18: mModes.TSM  = 1; break;
        case 19: mModes.EBM  = 1; break;
        case 20: mModes.LMN  = 1; break;
        case 21: mModes.GRCM = 1; break;
        case 22: mModes.ZDM  = 1; break;
        }
    });
}
// start of string
void SerialConnEcma48::ProcessSOS(const std::string_view& p)
{
}
// start of guarded area
void SerialConnEcma48::ProcessSPA(const std::string_view& p)
{
}
// select representation directions
void SerialConnEcma48::ProcessSPD(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessSPH(const std::string_view& p)
{
	int pn = atoi(p.data());
	if (pn <= 0) pn = 1;
	mPageHome = pn;
}
// spacing increment
void SerialConnEcma48::ProcessSPI(const std::string_view& p)
{
}
// set page limit
void SerialConnEcma48::ProcessSPL(const std::string_view& p)
{
}
// select print quality and rapidity
void SerialConnEcma48::ProcessSPQR(const std::string_view& p)
{
}
// scroll right
void SerialConnEcma48::ProcessSR(const std::string_view& p)
{
	int pn = atoi(p.data());
	if (pn <= 0) pn = 1;
	for (size_t k = 0; k < mLines.size(); ++k) {
		VTLine& vline = mLines[k];
		vline.insert(vline.begin(), pn, mBlankChar);
		for (int i = 0; i < pn; ++i)
			vline.pop_back();
	}
}
// start reversed string
void SerialConnEcma48::ProcessSRS(const std::string_view& p)
{
}
// set reduced character separation
void SerialConnEcma48::ProcessSRCS(const std::string_view& p)
{
}
// start of selected area
void SerialConnEcma48::ProcessSSA(const std::string_view& p)
{
}
// select size unit
void SerialConnEcma48::ProcessSSU(const std::string_view& p)
{
}
// set space width
void SerialConnEcma48::ProcessSSW(const std::string_view& p)
{
}
// single shift two
void SerialConnEcma48::ProcessSS2(const std::string_view& p)
{
}
// single shift three
void SerialConnEcma48::ProcessSS3(const std::string_view& p)
{
}
// string terminator
void SerialConnEcma48::ProcessST(const std::string_view& p)
{
}
// selective tabulation
void SerialConnEcma48::ProcessSTAB(const std::string_view& p)
{
}
// set transmit state
void SerialConnEcma48::ProcessSTS(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessSU(const std::string_view& p)
{
	int pn = atoi(p.data());
	if (pn <= 0) pn = 1;
	int top = mScrollingRegion.Top;
	int bot = mScrollingRegion.Bottom;
	if (bot < 0) bot = (int)mLines.size()-1;
	Size csz = GetConsoleSize();
	auto it_end = mLines.begin()+bot+1;
	mLines.insert(it_end, pn, VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
	mLines.erase(mLines.begin()+top, mLines.begin()+top+pn);
}
// select line spacing
void SerialConnEcma48::ProcessSVS(const std::string_view& p)
{
}
// tabulation aligned centred
void SerialConnEcma48::ProcessTAC(const std::string_view& p)
{
}
// tabulation aligned leading edge
void SerialConnEcma48::ProcessTALE(const std::string_view& p)
{
}
// tabulation aligned trailing edge
void SerialConnEcma48::ProcessTATE(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessTBC(const std::string_view& p)
{
	ProcessCTC(p);
}
// tabulation centred on character
void SerialConnEcma48::ProcessTCC(const std::string_view& p)
{
}
// tabulation stop remove
void SerialConnEcma48::ProcessTSR(const std::string_view& p)
{
}
// thin space specification
void SerialConnEcma48::ProcessTSS(const std::string_view& p)
{
}
void SerialConnEcma48::ProcessVPA(const std::string_view& p)
{
	// data component
	int pn = atoi(p.data());
	if (pn <= 0) pn = 1;
	mVy = pn-1;
}
void SerialConnEcma48::ProcessVPB(const std::string_view& p)
{
	// data component
	int pn = atoi(p.data());
	if (pn <= 0) pn = 1;
	mVy -= pn;
}
void SerialConnEcma48::ProcessVPR(const std::string_view& p)
{
	// data component
	int pn = atoi(p.data());
	if (pn <= 0) pn = 1;
	mVy += pn;
}
void SerialConnEcma48::ProcessVTS(const std::string_view& p)
{
	mLines[mVy][mVx] = '\v';
	mVy++;
}
bool SerialConnEcma48::ProcessChar(Upp::dword cc)
{
    std::vector<uint32_t> ss(1, cc);
    if (mModes.SRM == Ecma48Modes::SRM_Monitor) {
        RenderText(ss);
    }
    return SerialConnVT::ProcessChar(cc);
}
bool SerialConnEcma48::ProcessOverflowLines(const struct Seq& seq)
{
	if (seq.Type == Seq::CTRL_SEQ) {
		if (seq.Ctrl.first == ECMA48_LF || seq.Ctrl.first == ECMA48_FF || seq.Ctrl.first == ECMA48_VT)
			return SerialConnVT::ProcessOverflowLines(seq);
	}
	return false;
}

void SerialConnEcma48::SetUseS8C(bool b)
{
	mUseS8C = b;
}

static std::string S8CToS7C(const std::string& s)
{
	static const char* s7c[] = {
		"", "", "", "",
		"\E\x44", // IND
		"\E\x45", // NEL
		"\E\x46", // SSA
		"\E\x47", // ESA
		"\E\x48", // HTS
		"\E\x49", // HTJ
		"\E\x4a", // VTS
		"\E\x4b", // PLD
		"\E\x4c", // PLU
		"\E\x4d", // RI
		"\E\x4e", // SS2
		"\E\x4f", // SS3
		"\E\x50", // DCS
		"\E\x51", // PU1
		"\E\x52", // PU2
		"\E\x53", // STS
		"\E\x54", // CCH
		"\E\x55", // MW
		"\E\x56", // SPA
		"\E\x57", // EPA
		"", "",
		"\E\x5a", // DECID
		"\E\x5b", // CSI
		"\E\x5c", // ST
		"\E\x5d", // OSC
		"\E\x5e", // PM
		"\E\x5f"  // APC
	};
	std::string out;
	for (size_t k = 0; k < s.length(); ++k) {
		uint8_t c = (uint8_t)s[k];
		if (c >= 0x80 && c <= 0x9f) {
			out += s7c[c-0x80];
		} else {
			out.push_back(s[k]);
		}
	}
	return std::move(out);
}
static std::string S7CToS8C(const std::string& s)
{
	std::string out;
	for (size_t k = 0; k < s.length(); ) {
		if (s[k] == '\E') {
			if (k+1 < s.length()) {
				k++;
				switch ((uint8_t)s[k]) {
				case 0x44: out.push_back((char)(0x84)); break; // IND
				case 0x45: out.push_back((char)(0x85)); break; // NEL
				case 0x46: out.push_back((char)(0x86)); break; // SSA
				case 0x47: out.push_back((char)(0x87)); break; // ESA
				case 0x48: out.push_back((char)(0x88)); break; // HTS
				case 0x49: out.push_back((char)(0x89)); break; // THJ
				case 0x4a: out.push_back((char)(0x8a)); break; // VTS
				case 0x4b: out.push_back((char)(0x8b)); break; // PLD
				case 0x4c: out.push_back((char)(0x8c)); break; // PLU
				case 0x4d: out.push_back((char)(0x8d)); break; // RI
				case 0x4e: out.push_back((char)(0x8e)); break; // SS2
				case 0x4f: out.push_back((char)(0x8f)); break; // SS3
				case 0x50: out.push_back((char)(0x90)); break; // DCS
				case 0x51: out.push_back((char)(0x91)); break; // PU1
				case 0x52: out.push_back((char)(0x92)); break; // PU2
				case 0x53: out.push_back((char)(0x93)); break; // STS
				case 0x54: out.push_back((char)(0x94)); break; // CCH
				case 0x55: out.push_back((char)(0x95)); break; // MW
				case 0x56: out.push_back((char)(0x96)); break; // SPA
				case 0x57: out.push_back((char)(0x97)); break; // EPA
				case 0x5b: out.push_back((char)(0x9b)); break; // CSI
				case 0x5c: out.push_back((char)(0x9c)); break; // ST
				case 0x5d: out.push_back((char)(0x9d)); break; // OSC
				case 0x5e: out.push_back((char)(0x9e)); break; // PM
				case 0x5f: out.push_back((char)(0x9f)); break; // APC
				case 0x5a: out.push_back((char)(0x9a)); break; // DECID
				default:
					out.push_back(s[k]);
					break;
				}
			} else out.push_back(s[k]);
		} else out.push_back(s[k]);
		k++;
	}
	return std::move(out);
}

bool SerialConnEcma48::IsControlSeqPrefix(uint8_t c)
{
	if (mUseS8C) {
		return c <= 0x1f || c == 0x7f || (c >= 0x80 && c <= 0x9f);
	} else return SerialConnVT::IsControlSeqPrefix(c);
}

int SerialConnEcma48::IsControlSeq(const std::string_view& seq, size_t& p_begin, size_t& p_sz, size_t& s_end)
{
    if (seq[0] >= 0 && seq[0] < 0x20 && seq[0] != 0x1b) {
        p_begin = 0;
        p_sz = 1;
        s_end = 1;
        return ECMA48_NUL + seq[0];
    } else if (seq[0] == 0x1f) {
        p_begin = 0;
        p_sz = 1;
        s_end = 1;
        return ECMA48_DEL;
    }
    return SerialConnVT::IsControlSeq(seq, p_begin, p_sz, s_end);
}

void SerialConnEcma48::RefineTheInput(std::string& raw)
{
	if (mUseS8C) {
		raw = S8CToS7C(raw);
	}
}

void SerialConnEcma48::Put(const std::string& s)
{
	SerialConnVT::Put(mUseS8C ? S7CToS8C(s) : s);
}
