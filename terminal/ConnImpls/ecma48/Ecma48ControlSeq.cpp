//
// (c) 2020 chiv
//
#include "Ecma48ControlSeq.h"

static const char* kESC = "\x1b";
static const char* kCSI = "\x1bx5b";
//
static const char* kAPC = "\x1bx5f";
static const char* kCMD = "\x1bx64";
static const char* kDCS = "\x1bx50";
static const char* kOSC = "\x1bx5d";
static const char* kPM  = "\x1bx5e";
static const char* kSOS = "\x1bx58";
static const char* kST  = "\x1bx5c";

void AddEcma48ControlSeqs(ControlSeqFactory* factory)
{
    // C0
    REGISTER_SEQ(factory, ECMA48_SOH, "\x01", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_STX, "\x02", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_ETX, "\x03", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_EQT, "\x04", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_ENQ, "\x05", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_ACK, "\x06", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_BEL, "\x07", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_BS,  "\x08", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_HT,  "\x09", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_LF,  "\x0a", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_VT,  "\x0b", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_FF,  "\x0c", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_CR,  "\x0d", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_SO,  "\x0e", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_SI,  "\x0f", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_DLE, "\x10", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_DC1, "\x11", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_DC2, "\x12", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_DC3, "\x13", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_DC4, "\x14", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_NAK, "\x15", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_SYN, "\x16", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_ETB, "\x17", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_CAN, "\x18", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_EM,  "\x19", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_SUB, "\x1a", No, 0, "");
    // Ignore ESC itself
    REGISTER_SEQ(factory, ECMA48_FS, "\x1c", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_GS, "\x1d", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_RS, "\x1e", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_US, "\x1f", No, 0, "");
    // C1
    REGISTER_SEQ(factory, ECMA48_APC, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_BPH, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_CCH, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_CHA, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_CHT, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_CMD, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_CNL, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_CPL, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_CPR, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_CTC, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_CUB, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_CUD, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_CUF, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_CUP, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_CUF, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_CUU, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_CVT, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_DA, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_DAQ, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_DCH, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_DCS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_DL, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_DMI, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_DSR, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_DTA, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_EA, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_ECH, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_ED, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_EF, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_EL, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_EMI, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_EPA, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_ESA, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_FNK, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_FNT, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_GCC, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_GSM, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_GSS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_HPA, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_HPB, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_HPR, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_HTJ, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_HTS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_HVP, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_ICH, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_DCS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_IGS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_IL, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_INT, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_JFY, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_LS1R, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_LS2, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_LS2R, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_LS3, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_LS3R, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_MC, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_MW, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_NBH, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_NEL, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_NP, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_OSC, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_PEC, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_PFS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_PLD, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_PLU, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_PM, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_PP, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_PPA, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_PPB, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_PPR, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_PTX, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_PU1, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_PU2, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_QUAD, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_REP, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_RI, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_RIS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_RM, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SACS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SAPV, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SCI, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SCO, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SCP, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SCS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SD, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SDS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SEE, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SEF, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_GSR, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SHS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SIMD, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SL, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SLH, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SLL, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SLS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SM, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SOS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SPA, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SPD, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SPH, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SPI, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SPL, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SPQR, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SR, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SRCS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SSA, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SSU, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SSW, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SS2, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SS3, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_ST, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_STAB, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_STS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_STX, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SU, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_SVS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_TAC, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_TALE, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_TATE, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_TBC, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_TCC, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_TSR, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_TSS, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_VPA, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_VPB, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_VPR, kCSI, Pn, 1, "");
    REGISTER_SEQ(factory, ECMA48_VTS, kCSI, Pn, 1, "");
}
