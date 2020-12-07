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
{
	InstallFunctions();
}

SerialConnEcma48::~SerialConnEcma48()
{
}

void SerialConnEcma48::InstallFunctions()
{
    mFunctions[ECMA48_SOH] = [=](const std::string& p) { ProcessSOH(p); };
    mFunctions[ECMA48_STX] = [=](const std::string& p) { ProcessSTX(p); };
    mFunctions[ECMA48_ETX] = [=](const std::string& p) { ProcessETX(p); };
    mFunctions[ECMA48_EQT] = [=](const std::string& p) { ProcessEQT(p); };
    mFunctions[ECMA48_ENQ] = [=](const std::string& p) { ProcessENQ(p); };
    mFunctions[ECMA48_ACK] = [=](const std::string& p) { ProcessACK(p); };
    mFunctions[ECMA48_BEL] = [=](const std::string& p) { ProcessBEL(p); };
    mFunctions[ECMA48_BS] = [=](const std::string& p) { ProcessBS(p); };
    mFunctions[ECMA48_HT] = [=](const std::string& p) { ProcessHT(p); };
    mFunctions[ECMA48_LF] = [=](const std::string& p) { ProcessLF(p); };
    mFunctions[ECMA48_VT] = [=](const std::string& p) { ProcessVT(p); };
    mFunctions[ECMA48_FF] = [=](const std::string& p) { ProcessFF(p); };
    mFunctions[ECMA48_CR] = [=](const std::string& p) { ProcessCR(p); };
    mFunctions[ECMA48_SO] = [=](const std::string& p) { ProcessSO(p); };
    mFunctions[ECMA48_SI] = [=](const std::string& p) { ProcessSI(p); };
    mFunctions[ECMA48_DLE] = [=](const std::string& p) { ProcessDLE(p); };
    mFunctions[ECMA48_DC1] = [=](const std::string& p) { ProcessDC1(p); };
    mFunctions[ECMA48_DC2] = [=](const std::string& p) { ProcessDC2(p); };
    mFunctions[ECMA48_DC3] = [=](const std::string& p) { ProcessDC3(p); };
    mFunctions[ECMA48_DC4] = [=](const std::string& p) { ProcessDC4(p); };
    mFunctions[ECMA48_NAK] = [=](const std::string& p) { ProcessNAK(p); };
    mFunctions[ECMA48_SYN] = [=](const std::string& p) { ProcessSYN(p); };
    mFunctions[ECMA48_ETB] = [=](const std::string& p) { ProcessETB(p); };
    mFunctions[ECMA48_CAN] = [=](const std::string& p) { ProcessCAN(p); };
    mFunctions[ECMA48_EM] = [=](const std::string& p) { ProcessEM(p); };
    mFunctions[ECMA48_SUB] = [=](const std::string& p) { ProcessSUB(p); };
    mFunctions[ECMA48_FS] = [=](const std::string& p) { ProcessFS(p); };
    mFunctions[ECMA48_GS] = [=](const std::string& p) { ProcessGS(p); };
    mFunctions[ECMA48_RS] = [=](const std::string& p) { ProcessRS(p); };
    mFunctions[ECMA48_US] = [=](const std::string& p) { ProcessUS(p); };
    mFunctions[ECMA48_DEL] = [=](const std::string& p) { ProcessDEL(p); };
    // C1
    mFunctions[ECMA48_APC] = [=](const std::string& p) { ProcessAPC(p); };
    mFunctions[ECMA48_BPH] = [=](const std::string& p) { ProcessBPH(p); };
    mFunctions[ECMA48_CCH] = [=](const std::string& p) { ProcessCCH(p); };
    mFunctions[ECMA48_CHA] = [=](const std::string& p) { ProcessCHA(p); };
    mFunctions[ECMA48_CHT] = [=](const std::string& p) { ProcessCHT(p); };
    mFunctions[ECMA48_CMD] = [=](const std::string& p) { ProcessCMD(p); };
    mFunctions[ECMA48_CNL] = [=](const std::string& p) { ProcessCNL(p); };
    mFunctions[ECMA48_CPL] = [=](const std::string& p) { ProcessCPL(p); };
    mFunctions[ECMA48_CPR] = [=](const std::string& p) { ProcessCPR(p); };
    mFunctions[ECMA48_CTC] = [=](const std::string& p) { ProcessCTC(p); };
    mFunctions[ECMA48_CUB] = [=](const std::string& p) { ProcessCUB(p); };
    mFunctions[ECMA48_CUD] = [=](const std::string& p) { ProcessCUD(p); };
    mFunctions[ECMA48_CUF] = [=](const std::string& p) { ProcessCUF(p); };
    mFunctions[ECMA48_CUP] = [=](const std::string& p) { ProcessCUP(p); };
    mFunctions[ECMA48_CUF] = [=](const std::string& p) { ProcessCUF(p); };
    mFunctions[ECMA48_CUU] = [=](const std::string& p) { ProcessCUU(p); };
    mFunctions[ECMA48_CVT] = [=](const std::string& p) { ProcessCVT(p); };
    mFunctions[ECMA48_DA ] = [=](const std::string& p) { ProcessDA(p); };
    mFunctions[ECMA48_DAQ] = [=](const std::string& p) { ProcessDAQ(p); };
    mFunctions[ECMA48_DCH] = [=](const std::string& p) { ProcessDCH(p); };
    mFunctions[ECMA48_DCS] = [=](const std::string& p) { ProcessDCS(p); };
    mFunctions[ECMA48_DL ] = [=](const std::string& p) { ProcessDL(p); };
    mFunctions[ECMA48_DMI] = [=](const std::string& p) { ProcessDMI(p); };
    mFunctions[ECMA48_DSR] = [=](const std::string& p) { ProcessDSR(p); };
    mFunctions[ECMA48_DTA] = [=](const std::string& p) { ProcessDTA(p); };
    mFunctions[ECMA48_EA ] = [=](const std::string& p) { ProcessEA(p); };
    mFunctions[ECMA48_ECH] = [=](const std::string& p) { ProcessECH(p); };
    mFunctions[ECMA48_ED ] = [=](const std::string& p) { ProcessED(p); };
    mFunctions[ECMA48_EF ] = [=](const std::string& p) { ProcessEF(p); };
    mFunctions[ECMA48_EL ] = [=](const std::string& p) { ProcessEL(p); };
    mFunctions[ECMA48_EMI] = [=](const std::string& p) { ProcessEMI(p); };
    mFunctions[ECMA48_EPA] = [=](const std::string& p) { ProcessEPA(p); };
    mFunctions[ECMA48_ESA] = [=](const std::string& p) { ProcessESA(p); };
    mFunctions[ECMA48_FNK] = [=](const std::string& p) { ProcessFNK(p); };
    mFunctions[ECMA48_FNT] = [=](const std::string& p) { ProcessFNT(p); };
    mFunctions[ECMA48_GCC] = [=](const std::string& p) { ProcessGCC(p); };
    mFunctions[ECMA48_GSM] = [=](const std::string& p) { ProcessGSM(p); };
    mFunctions[ECMA48_GSS] = [=](const std::string& p) { ProcessGSS(p); };
    mFunctions[ECMA48_HPA] = [=](const std::string& p) { ProcessHPA(p); };
    mFunctions[ECMA48_HPB] = [=](const std::string& p) { ProcessHPB(p); };
    mFunctions[ECMA48_HPR] = [=](const std::string& p) { ProcessHPR(p); };
    mFunctions[ECMA48_HTJ] = [=](const std::string& p) { ProcessHTJ(p); };
    mFunctions[ECMA48_HTS] = [=](const std::string& p) { ProcessHTS(p); };
    mFunctions[ECMA48_HVP] = [=](const std::string& p) { ProcessHVP(p); };
    mFunctions[ECMA48_ICH] = [=](const std::string& p) { ProcessICH(p); };
    mFunctions[ECMA48_IDCS] = [=](const std::string& p) { ProcessIDCS(p); };
    mFunctions[ECMA48_IGS] = [=](const std::string& p) { ProcessIGS(p); };
    mFunctions[ECMA48_IL ] = [=](const std::string& p) { ProcessIL(p); };
    mFunctions[ECMA48_INT] = [=](const std::string& p) { ProcessINT(p); };
    mFunctions[ECMA48_JFY] = [=](const std::string& p) { ProcessJFY(p); };
    mFunctions[ECMA48_LS1R] = [=](const std::string& p) { ProcessLS1R(p); };
    mFunctions[ECMA48_LS2] = [=](const std::string& p) { ProcessLS2(p); };
    mFunctions[ECMA48_LS2R] = [=](const std::string& p) { ProcessLS2(p); };
    mFunctions[ECMA48_LS3] = [=](const std::string& p) { ProcessLS3(p); };
    mFunctions[ECMA48_LS3R] = [=](const std::string& p) { ProcessLS3(p); };
    mFunctions[ECMA48_MC ] = [=](const std::string& p) { ProcessMC(p); };
    mFunctions[ECMA48_MW ] = [=](const std::string& p) { ProcessMW(p); };
    mFunctions[ECMA48_NBH] = [=](const std::string& p) { ProcessNBH(p); };
    mFunctions[ECMA48_NEL] = [=](const std::string& p) { ProcessNEL(p); };
    mFunctions[ECMA48_NP ] = [=](const std::string& p) { ProcessNP(p); };
    mFunctions[ECMA48_OSC] = [=](const std::string& p) { ProcessOSC(p); };
    mFunctions[ECMA48_PEC] = [=](const std::string& p) { ProcessPEC(p); };
    mFunctions[ECMA48_PFS] = [=](const std::string& p) { ProcessPFS(p); };
    mFunctions[ECMA48_PLD] = [=](const std::string& p) { ProcessPLD(p); };
    mFunctions[ECMA48_PLU] = [=](const std::string& p) { ProcessPLU(p); };
    mFunctions[ECMA48_PM ] = [=](const std::string& p) { ProcessPM(p); };
    mFunctions[ECMA48_PP ] = [=](const std::string& p) { ProcessPP(p); };
    mFunctions[ECMA48_PPA] = [=](const std::string& p) { ProcessPPA(p); };
    mFunctions[ECMA48_PPB] = [=](const std::string& p) { ProcessPPB(p); };
    mFunctions[ECMA48_PPR] = [=](const std::string& p) { ProcessPPR(p); };
    mFunctions[ECMA48_PTX] = [=](const std::string& p) { ProcessPTX(p); };
    mFunctions[ECMA48_PU1] = [=](const std::string& p) { ProcessPU1(p); };
    mFunctions[ECMA48_PU2] = [=](const std::string& p) { ProcessPU2(p); };
    mFunctions[ECMA48_QUAD] = [=](const std::string& p) { ProcessQUAD(p); };
    mFunctions[ECMA48_REP] = [=](const std::string& p) { ProcessREP(p); };
    mFunctions[ECMA48_RI ] = [=](const std::string& p) { ProcessRI(p); };
    mFunctions[ECMA48_RIS] = [=](const std::string& p) { ProcessRIS(p); };
    mFunctions[ECMA48_RM ] = [=](const std::string& p) { ProcessRM(p); };
    mFunctions[ECMA48_SACS] = [=](const std::string& p) { ProcessSACS(p); };
    mFunctions[ECMA48_SAPV] = [=](const std::string& p) { ProcessSAPV(p); };
    mFunctions[ECMA48_SCI] = [=](const std::string& p) { ProcessSCI(p); };
    mFunctions[ECMA48_SCO] = [=](const std::string& p) { ProcessSCO(p); };
    mFunctions[ECMA48_SCP] = [=](const std::string& p) { ProcessSCP(p); };
    mFunctions[ECMA48_SCS] = [=](const std::string& p) { ProcessSCS(p); };
    mFunctions[ECMA48_SD ] = [=](const std::string& p) { ProcessSD(p); };
    mFunctions[ECMA48_SDS] = [=](const std::string& p) { ProcessSDS(p); };
    mFunctions[ECMA48_SEE] = [=](const std::string& p) { ProcessSEE(p); };
    mFunctions[ECMA48_SEF] = [=](const std::string& p) { ProcessSEF(p); };
    mFunctions[ECMA48_GSR] = [=](const std::string& p) { ProcessGSR(p); };
    mFunctions[ECMA48_SHS] = [=](const std::string& p) { ProcessSHS(p); };
    mFunctions[ECMA48_SIMD] = [=](const std::string& p) { ProcessSIMD(p); };
    mFunctions[ECMA48_SL ] = [=](const std::string& p) { ProcessSL(p); };
    mFunctions[ECMA48_SLH] = [=](const std::string& p) { ProcessSLH(p); };
    mFunctions[ECMA48_SLL] = [=](const std::string& p) { ProcessSLL(p); };
    mFunctions[ECMA48_SLS] = [=](const std::string& p) { ProcessSLS(p); };
    mFunctions[ECMA48_SM ] = [=](const std::string& p) { ProcessSM(p); };
    mFunctions[ECMA48_SOS] = [=](const std::string& p) { ProcessSOS(p); };
    mFunctions[ECMA48_SPA] = [=](const std::string& p) { ProcessSPA(p); };
    mFunctions[ECMA48_SPD] = [=](const std::string& p) { ProcessSPD(p); };
    mFunctions[ECMA48_SPH] = [=](const std::string& p) { ProcessSPH(p); };
    mFunctions[ECMA48_SPI] = [=](const std::string& p) { ProcessSPI(p); };
    mFunctions[ECMA48_SPL] = [=](const std::string& p) { ProcessSPL(p); };
    mFunctions[ECMA48_SPQR] = [=](const std::string& p) { ProcessSPQR(p); };
    mFunctions[ECMA48_SR ] = [=](const std::string& p) { ProcessSR(p); };
    mFunctions[ECMA48_SRCS] = [=](const std::string& p) { ProcessSRCS(p); };
    mFunctions[ECMA48_SSA] = [=](const std::string& p) { ProcessSSA(p); };
    mFunctions[ECMA48_SSU] = [=](const std::string& p) { ProcessSSU(p); };
    mFunctions[ECMA48_SSW] = [=](const std::string& p) { ProcessSSW(p); };
    mFunctions[ECMA48_SS2] = [=](const std::string& p) { ProcessSS2(p); };
    mFunctions[ECMA48_SS3] = [=](const std::string& p) { ProcessSS3(p); };
    mFunctions[ECMA48_ST ] = [=](const std::string& p) { ProcessST(p); };
    mFunctions[ECMA48_STAB] = [=](const std::string& p) { ProcessSTAB(p); };
    mFunctions[ECMA48_STS] = [=](const std::string& p) { ProcessSTS(p); };
    mFunctions[ECMA48_STX] = [=](const std::string& p) { ProcessSTX(p); };
    mFunctions[ECMA48_SU ] = [=](const std::string& p) { ProcessSU(p); };
    mFunctions[ECMA48_SVS] = [=](const std::string& p) { ProcessSVS(p); };
    mFunctions[ECMA48_TAC] = [=](const std::string& p) { ProcessTAC(p); };
    mFunctions[ECMA48_TALE] = [=](const std::string& p) { ProcessTALE(p); };
    mFunctions[ECMA48_TATE] = [=](const std::string& p) { ProcessTATE(p); };
    mFunctions[ECMA48_TBC] = [=](const std::string& p) { ProcessTBC(p); };
    mFunctions[ECMA48_TCC] = [=](const std::string& p) { ProcessTCC(p); };
    mFunctions[ECMA48_TSR] = [=](const std::string& p) { ProcessTSR(p); };
    mFunctions[ECMA48_TSS] = [=](const std::string& p) { ProcessTSS(p); };
    mFunctions[ECMA48_VPA] = [=](const std::string& p) { ProcessVPA(p); };
    mFunctions[ECMA48_VPB] = [=](const std::string& p) { ProcessVPB(p); };
    mFunctions[ECMA48_VPR] = [=](const std::string& p) { ProcessVPR(p); };
    mFunctions[ECMA48_VTS] = [=](const std::string& p) { ProcessVTS(p); };
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
//
void SerialConnEcma48::ProcessSOH(const std::string&)
{
}
void SerialConnEcma48::ProcessSTX(const std::string&)
{
}
void SerialConnEcma48::ProcessETX(const std::string&)
{
}
void SerialConnEcma48::ProcessEQT(const std::string&)
{
}
void SerialConnEcma48::ProcessENQ(const std::string&)
{
}
void SerialConnEcma48::ProcessACK(const std::string&)
{
}
void SerialConnEcma48::ProcessBEL(const std::string&)
{
}
void SerialConnEcma48::ProcessBS(const std::string&)
{
	mPx -= mFontW;
}
void SerialConnEcma48::ProcessHT(const std::string&)
{
	int tabsz = mFontW*8;
	mPx += tabsz - (mPx % tabsz);
}
void SerialConnEcma48::ProcessLF(const std::string&)
{
	mVy += 1;
}
void SerialConnEcma48::ProcessVT(const std::string&)
{
	mVy += 1;
}
void SerialConnEcma48::ProcessFF(const std::string&)
{
	mVy += 1;
}
void SerialConnEcma48::ProcessCR(const std::string&)
{
	mVx = 0;
}
void SerialConnEcma48::ProcessSO(const std::string&)
{
}
void SerialConnEcma48::ProcessSI(const std::string&)
{
}
void SerialConnEcma48::ProcessDLE(const std::string&)
{
}
void SerialConnEcma48::ProcessDC1(const std::string&)
{
}
void SerialConnEcma48::ProcessDC2(const std::string&)
{
}
void SerialConnEcma48::ProcessDC3(const std::string&)
{
}
void SerialConnEcma48::ProcessDC4(const std::string&)
{
}
void SerialConnEcma48::ProcessNAK(const std::string&)
{
}
void SerialConnEcma48::ProcessSYN(const std::string&)
{
}
void SerialConnEcma48::ProcessETB(const std::string&)
{
}
void SerialConnEcma48::ProcessCAN(const std::string&)
{
}
void SerialConnEcma48::ProcessEM(const std::string&)
{
}
void SerialConnEcma48::ProcessSUB(const std::string&)
{
}

void SerialConnEcma48::ProcessFS(const std::string&)
{
}
void SerialConnEcma48::ProcessGS(const std::string&)
{
}
void SerialConnEcma48::ProcessRS(const std::string&)
{
}
void SerialConnEcma48::ProcessUS(const std::string&)
{
}
void SerialConnEcma48::ProcessDEL(const std::string&)
{
}
void SerialConnEcma48::ProcessAPC(const std::string& p)
{
}
void SerialConnEcma48::ProcessBPH(const std::string& p)
{
}
void SerialConnEcma48::ProcessCCH(const std::string& p)
{
}
void SerialConnEcma48::ProcessCHA(const std::string& p)
{
}
void SerialConnEcma48::ProcessCHT(const std::string& p)
{
}
void SerialConnEcma48::ProcessCMD(const std::string& p)
{
}
void SerialConnEcma48::ProcessCNL(const std::string& p)
{
}
void SerialConnEcma48::ProcessCPL(const std::string& p)
{
}
void SerialConnEcma48::ProcessCPR(const std::string& p)
{
}
void SerialConnEcma48::ProcessCTC(const std::string& p)
{
}
void SerialConnEcma48::ProcessCUB(const std::string& p)
{
}
void SerialConnEcma48::ProcessCUD(const std::string& p)
{
}
void SerialConnEcma48::ProcessCUF(const std::string& p)
{
}
void SerialConnEcma48::ProcessCUP(const std::string& p)
{
}
void SerialConnEcma48::ProcessCUU(const std::string& p)
{
}
void SerialConnEcma48::ProcessCVT(const std::string& p)
{
}
void SerialConnEcma48::ProcessDA(const std::string& p)
{
}
void SerialConnEcma48::ProcessDAQ(const std::string& p)
{
}
void SerialConnEcma48::ProcessDCH(const std::string& p)
{
}
void SerialConnEcma48::ProcessDCS(const std::string& p)
{
}
void SerialConnEcma48::ProcessDL(const std::string& p)
{
}
void SerialConnEcma48::ProcessDMI(const std::string& p)
{
}
void SerialConnEcma48::ProcessDSR(const std::string& p)
{
}
void SerialConnEcma48::ProcessDTA(const std::string& p)
{
}
void SerialConnEcma48::ProcessEA(const std::string& p)
{
}
void SerialConnEcma48::ProcessECH(const std::string& p)
{
}
void SerialConnEcma48::ProcessED(const std::string& p)
{
}
void SerialConnEcma48::ProcessEF(const std::string& p)
{
}
void SerialConnEcma48::ProcessEL(const std::string& p)
{
}
void SerialConnEcma48::ProcessEMI(const std::string& p)
{
}
void SerialConnEcma48::ProcessEPA(const std::string& p)
{
}
void SerialConnEcma48::ProcessESA(const std::string& p)
{
}
void SerialConnEcma48::ProcessFNK(const std::string& p)
{
}
void SerialConnEcma48::ProcessFNT(const std::string& p)
{
}
void SerialConnEcma48::ProcessGCC(const std::string& p)
{
}
void SerialConnEcma48::ProcessGSM(const std::string& p)
{
}
void SerialConnEcma48::ProcessGSS(const std::string& p)
{
}
void SerialConnEcma48::ProcessHPA(const std::string& p)
{
}
void SerialConnEcma48::ProcessHPB(const std::string& p)
{
}
void SerialConnEcma48::ProcessHPR(const std::string& p)
{
}
void SerialConnEcma48::ProcessHTJ(const std::string& p)
{
}
void SerialConnEcma48::ProcessHTS(const std::string& p)
{
}
void SerialConnEcma48::ProcessHVP(const std::string& p)
{
}
void SerialConnEcma48::ProcessICH(const std::string& p)
{
}
void SerialConnEcma48::ProcessIDCS(const std::string& p)
{
}
void SerialConnEcma48::ProcessIGS(const std::string& p)
{
}
void SerialConnEcma48::ProcessIL(const std::string& p)
{
}
void SerialConnEcma48::ProcessINT(const std::string& p)
{
}
void SerialConnEcma48::ProcessJFY(const std::string& p)
{
}
void SerialConnEcma48::ProcessLS1R(const std::string& p)
{
}
void SerialConnEcma48::ProcessLS2(const std::string& p)
{
}
void SerialConnEcma48::ProcessLS2R(const std::string& p)
{
}
void SerialConnEcma48::ProcessLS3(const std::string& p)
{
}
void SerialConnEcma48::ProcessLS3R(const std::string& p)
{
}
void SerialConnEcma48::ProcessMC(const std::string& p)
{
}
void SerialConnEcma48::ProcessMW(const std::string& p)
{
}
void SerialConnEcma48::ProcessNBH(const std::string& p)
{
}
void SerialConnEcma48::ProcessNEL(const std::string& p)
{
}
void SerialConnEcma48::ProcessNP(const std::string& p)
{
}
void SerialConnEcma48::ProcessOSC(const std::string& p)
{
}
void SerialConnEcma48::ProcessPEC(const std::string& p)
{
}
void SerialConnEcma48::ProcessPFS(const std::string& p)
{
}
void SerialConnEcma48::ProcessPLD(const std::string& p)
{
}
void SerialConnEcma48::ProcessPLU(const std::string& p)
{
}
void SerialConnEcma48::ProcessPM(const std::string& p)
{
}
void SerialConnEcma48::ProcessPP(const std::string& p)
{
}
void SerialConnEcma48::ProcessPPA(const std::string& p)
{
}
void SerialConnEcma48::ProcessPPB(const std::string& p)
{
}
void SerialConnEcma48::ProcessPPR(const std::string& p)
{
}
void SerialConnEcma48::ProcessPTX(const std::string& p)
{
}
void SerialConnEcma48::ProcessPU1(const std::string& p)
{
}
void SerialConnEcma48::ProcessPU2(const std::string& p)
{
}
void SerialConnEcma48::ProcessQUAD(const std::string& p)
{
}
void SerialConnEcma48::ProcessREP(const std::string& p)
{
}
void SerialConnEcma48::ProcessRI(const std::string& p)
{
}
void SerialConnEcma48::ProcessRIS(const std::string& p)
{
}
void SerialConnEcma48::ProcessRM(const std::string& p)
{
}
void SerialConnEcma48::ProcessSACS(const std::string& p)
{
}
void SerialConnEcma48::ProcessSAPV(const std::string& p)
{
}
void SerialConnEcma48::ProcessSCI(const std::string& p)
{
}
void SerialConnEcma48::ProcessSCO(const std::string& p)
{
}
void SerialConnEcma48::ProcessSCP(const std::string& p)
{
}
void SerialConnEcma48::ProcessSCS(const std::string& p)
{
}
void SerialConnEcma48::ProcessSD(const std::string& p)
{
}
void SerialConnEcma48::ProcessSDS(const std::string& p)
{
}
void SerialConnEcma48::ProcessSEE(const std::string& p)
{
}
void SerialConnEcma48::ProcessSEF(const std::string& p)
{
}
void SerialConnEcma48::ProcessGSR(const std::string& p)
{
}
void SerialConnEcma48::ProcessSHS(const std::string& p)
{
}
void SerialConnEcma48::ProcessSIMD(const std::string& p)
{
}
void SerialConnEcma48::ProcessSL(const std::string& p)
{
}
void SerialConnEcma48::ProcessSLH(const std::string& p)
{
}
void SerialConnEcma48::ProcessSLL(const std::string& p)
{
}
void SerialConnEcma48::ProcessSLS(const std::string& p)
{
}
void SerialConnEcma48::ProcessSM(const std::string& p)
{
}
void SerialConnEcma48::ProcessSOS(const std::string& p)
{
}
void SerialConnEcma48::ProcessSPA(const std::string& p)
{
}
void SerialConnEcma48::ProcessSPD(const std::string& p)
{
}
void SerialConnEcma48::ProcessSPH(const std::string& p)
{
}
void SerialConnEcma48::ProcessSPI(const std::string& p)
{
}
void SerialConnEcma48::ProcessSPL(const std::string& p)
{
}
void SerialConnEcma48::ProcessSPQR(const std::string& p)
{
}
void SerialConnEcma48::ProcessSR(const std::string& p)
{
}
void SerialConnEcma48::ProcessSRCS(const std::string& p)
{
}
void SerialConnEcma48::ProcessSSA(const std::string& p)
{
}
void SerialConnEcma48::ProcessSSU(const std::string& p)
{
}
void SerialConnEcma48::ProcessSSW(const std::string& p)
{
}
void SerialConnEcma48::ProcessSS2(const std::string& p)
{
}
void SerialConnEcma48::ProcessSS3(const std::string& p)
{
}
void SerialConnEcma48::ProcessST(const std::string& p)
{
}
void SerialConnEcma48::ProcessSTAB(const std::string& p)
{
}
void SerialConnEcma48::ProcessSTS(const std::string& p)
{
}
void SerialConnEcma48::ProcessSU(const std::string& p)
{
}
void SerialConnEcma48::ProcessSVS(const std::string& p)
{
}
void SerialConnEcma48::ProcessTAC(const std::string& p)
{
}
void SerialConnEcma48::ProcessTALE(const std::string& p)
{
}
void SerialConnEcma48::ProcessTATE(const std::string& p)
{
}
void SerialConnEcma48::ProcessTBC(const std::string& p)
{
}
void SerialConnEcma48::ProcessTCC(const std::string& p)
{
}
void SerialConnEcma48::ProcessTSR(const std::string& p)
{
}
void SerialConnEcma48::ProcessTSS(const std::string& p)
{
}
void SerialConnEcma48::ProcessVPA(const std::string& p)
{
}
void SerialConnEcma48::ProcessVPB(const std::string& p)
{
}
void SerialConnEcma48::ProcessVPR(const std::string& p)
{
}
void SerialConnEcma48::ProcessVTS(const std::string& p)
{
}