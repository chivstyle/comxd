//
// (c) 2020 chiv
//
#include "Ecma48ControlSeq.h"

#define kESC   "\x1b"
#define kCSI   "\x1b\x5b"
#define kAPC   "\x1b\x5f"
#define kCMD   "\x1b\x64"
#define kDCS   "\x1b\x50"
#define kOSC   "\x1b\x5d"
#define kPM    "\x1b\x5e"
#define kSOS   "\x1b\x58"
#define kST    "\x1b\x5c"

void AddEcma48ControlSeqs(ControlSeqFactory* factory)
{
    REGISTER_SEQ(factory, ECMA48_ACK, "\x06", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_APC, kAPC,   Gs, 0, kST);
    REGISTER_SEQ(factory, ECMA48_BEL, "\x07", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_BPH, kESC,   No, 0, "\x42");
    REGISTER_SEQ(factory, ECMA48_BS,  "\x08", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_CAN, "\x18", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_CBT, kCSI,   Pn, 0, "\x5a");
    REGISTER_SEQ(factory, ECMA48_CCH, kESC,   No, 0, "\x54");
    REGISTER_SEQ(factory, ECMA48_CHA, kCSI,   Pn, 0, "\x47");
    REGISTER_SEQ(factory, ECMA48_CHT, kCSI,   Pn, 0, "\x49");
    REGISTER_SEQ(factory, ECMA48_CMD, kCSI,   No, 0, "\x64");
    REGISTER_SEQ(factory, ECMA48_CNL, kCSI,   Pn, 0, "\x45");
    REGISTER_SEQ(factory, ECMA48_CPL, kCSI,   Pn, 0, "\x46");
    REGISTER_SEQ(factory, ECMA48_CPR, kCSI,   Pn, 0, "\x52");
    REGISTER_SEQ(factory, ECMA48_CR,  "\x0d", No, 0, "");
    // HERE is CSI
    REGISTER_SEQ(factory, ECMA48_CTC, kCSI, Ps, 0, "\x57");
    REGISTER_SEQ(factory, ECMA48_CUB, kCSI, Pn, 0, "\x44");
    REGISTER_SEQ(factory, ECMA48_CUD, kCSI, Pn, 0, "\x42");
    REGISTER_SEQ(factory, ECMA48_CUF, kCSI, Pn, 0, "\x43");
    REGISTER_SEQ(factory, ECMA48_CUP, kCSI, Pn, 0, "\x48");
    REGISTER_SEQ(factory, ECMA48_CUU, kCSI, Pn, 0, "\x41");
    REGISTER_SEQ(factory, ECMA48_CVT, kCSI, Pn, 0, "\x59");
    REGISTER_SEQ(factory, ECMA48_DA,  kCSI, Ps, 0, "\x63");
    REGISTER_SEQ(factory, ECMA48_DAQ, kCSI, Ps, 0, "\x6f");
    REGISTER_SEQ(factory, ECMA48_DCH, kCSI, Pn, 0, "\x50");
    REGISTER_SEQ(factory, ECMA48_DCS, kESC, No, 0, "\x50");
    REGISTER_SEQ(factory, ECMA48_DC1, "\x11", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_DC2, "\x12", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_DC3, "\x13", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_DC4, "\x14", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_DL,  kCSI,   Pn, 0, "\x4d");
    REGISTER_SEQ(factory, ECMA48_DLE, "\x10", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_DMI, kESC,   No, 0, "\x60");
    REGISTER_SEQ(factory, ECMA48_DSR, kCSI,   Ps, 0, "\x6e");
    REGISTER_SEQ(factory, ECMA48_DTA, kCSI,   Pn, 0, "\x20\x54");
    REGISTER_SEQ(factory, ECMA48_EA,  kCSI,   Ps, 0, "\x4f");
    REGISTER_SEQ(factory, ECMA48_ECH, kCSI,   Pn, 0, "\x58");
    REGISTER_SEQ(factory, ECMA48_ED,   kCSI,  Ps, 0, "\x4a");
    REGISTER_SEQ(factory, ECMA48_EF,   kCSI,  Ps, 0, "\x4e");
    REGISTER_SEQ(factory, ECMA48_EL,   kCSI,  Ps, 0, "\x4b");
    REGISTER_SEQ(factory, ECMA48_EM,  "\x19", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_EMI, kESC,   No, 0, "\x62");
    REGISTER_SEQ(factory, ECMA48_ENQ, "\x05", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_EOT, "\x04", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_EPA, kESC,   No, 0, "\x57");
    REGISTER_SEQ(factory, ECMA48_ESA, kESC,   No, 0, "\x47");
    // HERE is ESC
    REGISTER_SEQ(factory, ECMA48_ETB, "\x17", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_ETX, "\x03", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_FF,  "\x0c", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_FNK, kCSI,   Pn, 0, "\x20\x57");
    REGISTER_SEQ(factory, ECMA48_FNT, kCSI,   Pn, 0, "\x20\x44");
    REGISTER_SEQ(factory, ECMA48_GCC, kCSI,   Ps, 0, "\x20\x5f");
    REGISTER_SEQ(factory, ECMA48_GSM, kCSI,   Pn, 0, "\x20\x42");
    REGISTER_SEQ(factory, ECMA48_GSS, kCSI,   Pn, 0, "\x20\x43");
    REGISTER_SEQ(factory, ECMA48_HPA, kCSI,   Pn, 0, "\x60");
    REGISTER_SEQ(factory, ECMA48_HPB, kCSI,   Pn, 0, "\x6a");
    REGISTER_SEQ(factory, ECMA48_HPR, kCSI,   Pn, 0, "\x61");
    REGISTER_SEQ(factory, ECMA48_HT,  "\x09", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_HTJ, kESC,   No, 0, "\x49");
    REGISTER_SEQ(factory, ECMA48_HTS, kESC,   No, 0, "\x48");
    REGISTER_SEQ(factory, ECMA48_HVP, kCSI,   Pn, 0, "\x66");
    REGISTER_SEQ(factory, ECMA48_ICH, kCSI,   Pn, 0, "\x40");
    REGISTER_SEQ(factory, ECMA48_IDCS, kCSI,  Ps, 0, "\x20\x4f");
    REGISTER_SEQ(factory, ECMA48_IGS, kCSI,   Ps, 0, "\x20\x4d");
    REGISTER_SEQ(factory, ECMA48_IL,  kCSI,   Pn, 0, "\x4c");
    REGISTER_SEQ(factory, ECMA48_INT, kESC,   No, 0, "\x61");
    REGISTER_SEQ(factory, ECMA48_US, "\x1f",  No, 0, ""); // IS1
    REGISTER_SEQ(factory, ECMA48_RS, "\x1e",  No, 0, ""); // IS2
    REGISTER_SEQ(factory, ECMA48_GS, "\x1d",  No, 0, ""); // IS3
    REGISTER_SEQ(factory, ECMA48_FS, "\x1c",  No, 0, ""); // IS4
    REGISTER_SEQ(factory, ECMA48_JFY, kCSI,   Ps, 0, "\x20\x46");
    REGISTER_SEQ(factory, ECMA48_LF,  "\x0a", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_SI,  "\x0f", No, 0, ""); // LS0
    REGISTER_SEQ(factory, ECMA48_SO,  "\x0e", No, 0, ""); // LS1
    REGISTER_SEQ(factory, ECMA48_LS1R, kESC,  No, 0, "\x7e");
    REGISTER_SEQ(factory, ECMA48_LS2,  kESC,  No, 0, "\x6e");
    REGISTER_SEQ(factory, ECMA48_LS2R, kESC,  No, 0, "\x7d");
    REGISTER_SEQ(factory, ECMA48_LS3,  kESC,  No, 0, "\x6f");
    REGISTER_SEQ(factory, ECMA48_LS3R, kESC,  No, 0, "\x7c");
    REGISTER_SEQ(factory, ECMA48_MC,   kCSI,  Ps, 0, "\x69");
    REGISTER_SEQ(factory, ECMA48_MW,   kESC,  No, 0, "\x55");
    REGISTER_SEQ(factory, ECMA48_NAK, "\x15", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_NBH, kESC,   No, 0, "\x43");
    REGISTER_SEQ(factory, ECMA48_NEL, kESC,   No, 0, "\x45");
    REGISTER_SEQ(factory, ECMA48_NP,  kCSI,   Pn, 0, "\x55");
    // HERE is NUL, ignore it.
    REGISTER_SEQ(factory, ECMA48_OSC,  kOSC, Gs, 0, kST);
    REGISTER_SEQ(factory, ECMA48_PEC,  kCSI, Ps, 0, "\x20\x5a");
    REGISTER_SEQ(factory, ECMA48_PFS,  kCSI, Ps, 0, "\x20\x4a");
    REGISTER_SEQ(factory, ECMA48_PLD,  kESC, No, 0, "\x4b");
    REGISTER_SEQ(factory, ECMA48_PLU,  kESC, No, 0, "\x4c");
    REGISTER_SEQ(factory, ECMA48_PM,   kESC, No, 0, "\x5e");
    REGISTER_SEQ(factory, ECMA48_PP,   kCSI, Pn, 0, "\x56");
    REGISTER_SEQ(factory, ECMA48_PPA,  kCSI, Pn, 0, "\x20\x50");
    REGISTER_SEQ(factory, ECMA48_PPB,  kCSI, Pn, 0, "\x20\x52");
    REGISTER_SEQ(factory, ECMA48_PPR,  kCSI, Pn, 0, "\x20\x51");
    REGISTER_SEQ(factory, ECMA48_PTX,  kCSI, Ps, 0, "\x5c");
    REGISTER_SEQ(factory, ECMA48_PU1,  kESC, No, 0, "\x51");
    REGISTER_SEQ(factory, ECMA48_PU2,  kESC, No, 0, "\x52");
    REGISTER_SEQ(factory, ECMA48_QUAD, kCSI, Ps, 0, "\x20\x48");
    REGISTER_SEQ(factory, ECMA48_REP,  kCSI, Pn, 0, "\x62");
    REGISTER_SEQ(factory, ECMA48_RI,   kESC, No, 0, "\x4d");
    REGISTER_SEQ(factory, ECMA48_RIS,  kESC, No, 0, "\x63");
    REGISTER_SEQ(factory, ECMA48_RM,   kCSI, Ps, 0, "\x6c");
    REGISTER_SEQ(factory, ECMA48_SACS, kCSI, Pn, 0, "\x20\x5c");
    REGISTER_SEQ(factory, ECMA48_SAPV, kCSI, Ps, 0, "\x20\x5d");
    REGISTER_SEQ(factory, ECMA48_SCI,  kESC, No, 0, "\x5a");
    REGISTER_SEQ(factory, ECMA48_SCO,  kCSI, Ps, 0, "\x20\x65");
    REGISTER_SEQ(factory, ECMA48_SCP,  kCSI, Ps, 0, "\x20\x6b");
    REGISTER_SEQ(factory, ECMA48_SCS,  kCSI, Pn, 0, "\x20\x67");
    REGISTER_SEQ(factory, ECMA48_SD,   kCSI, Pn, 0, "\x54");
    REGISTER_SEQ(factory, ECMA48_SDS,  kCSI, Ps, 0, "\x5d");
    REGISTER_SEQ(factory, ECMA48_SEE,  kCSI, Ps, 0, "\x51");
    REGISTER_SEQ(factory, ECMA48_SEF,  kCSI, Ps, 0, "\x20\x59");
    REGISTER_SEQ(factory, ECMA48_SGR,  kCSI, Ps, 0, "\x6d");
    REGISTER_SEQ(factory, ECMA48_SHS,  kCSI, Ps, 0, "\x20\x4b");
    // HERE is C0, 0x0f, added before
    REGISTER_SEQ(factory, ECMA48_SIMD, kCSI, Ps, 0, "\x5e");
    REGISTER_SEQ(factory, ECMA48_SL,   kCSI, Pn, 0, "\x20\x40");
    REGISTER_SEQ(factory, ECMA48_SLH,  kCSI, Pn, 0, "\x20\x55");
    REGISTER_SEQ(factory, ECMA48_SLL,  kCSI, Pn, 0, "\x20\x56");
    REGISTER_SEQ(factory, ECMA48_SLS,  kCSI, Pn, 0, "\x20\x68");
    REGISTER_SEQ(factory, ECMA48_SM,   kCSI, Ps, 0, "\x68");
    // HERE is C0, 0x0e, added before
    REGISTER_SEQ(factory, ECMA48_SOH,  "\x01", No, 0, "");
    REGISTER_SEQ(factory, ECMA48_SOS,  kSOS,   Gs, 0, kST);
    REGISTER_SEQ(factory, ECMA48_SPA,  kESC,   No, 0, "\x56");
    REGISTER_SEQ(factory, ECMA48_SPD,  kCSI,   Ps, 0, "\x20\x53");
    REGISTER_SEQ(factory, ECMA48_SPH,  kCSI,   Pn, 0, "\x20\x69");
    REGISTER_SEQ(factory, ECMA48_SPI,  kCSI,   Pn, 0, "\x20\x47");
    REGISTER_SEQ(factory, ECMA48_SPL,  kCSI,   Pn, 0, "\x20\x6a");
    REGISTER_SEQ(factory, ECMA48_SPQR, kCSI,   Ps, 0, "\x20\x58");
    REGISTER_SEQ(factory, ECMA48_SR,   kCSI,   Pn, 0, "\x20\x41");
    REGISTER_SEQ(factory, ECMA48_SRCS, kCSI,   Pn, 0, "\x20\x66");
    REGISTER_SEQ(factory, ECMA48_SRS,  kCSI,   Ps, 0, "\x5b");
    REGISTER_SEQ(factory, ECMA48_SSA,  kESC,   No, 0, "\x46");
    REGISTER_SEQ(factory, ECMA48_SSU,  kCSI,   Ps, 0, "\x20\x49");
    REGISTER_SEQ(factory, ECMA48_SSW,  kCSI,   Pn, 0, "\x20\x5b");
    REGISTER_SEQ(factory, ECMA48_SS2,  kESC,   No, 0, "\x4e");
    REGISTER_SEQ(factory, ECMA48_SS3,  kESC,   No, 0, "\x4f");
    REGISTER_SEQ(factory, ECMA48_ST,   kST,    No, 0, "");
    REGISTER_SEQ(factory, ECMA48_STAB, kCSI,   Ps, 0, "\x20\x5e");
    REGISTER_SEQ(factory, ECMA48_STS,  kESC,   No, 0, "\x53");
    REGISTER_SEQ(factory, ECMA48_STX, "\x02",  No, 0, "");
    REGISTER_SEQ(factory, ECMA48_SU,  kCSI,    Pn, 0, "\x53");
    REGISTER_SEQ(factory, ECMA48_SUB, "\x1a",  No, 0, "");
    REGISTER_SEQ(factory, ECMA48_SVS, kCSI,    Ps, 0, "\x20\x4c");
    REGISTER_SEQ(factory, ECMA48_SYN, "\x16",  No, 0, "");
    REGISTER_SEQ(factory, ECMA48_TAC,  kCSI,   Pn, 0, "\x20\x62");
    REGISTER_SEQ(factory, ECMA48_TALE, kCSI,   Pn, 0, "\x20\x61");
    REGISTER_SEQ(factory, ECMA48_TATE, kCSI,   Pn, 0, "\x20\x60");
    REGISTER_SEQ(factory, ECMA48_TBC,  kCSI,   Ps, 0, "\x67");
    REGISTER_SEQ(factory, ECMA48_TCC,  kCSI,   Pn, 0, "\x20\x63");
    REGISTER_SEQ(factory, ECMA48_TSR,  kCSI,   Pn, 0, "\x20\x64");
    REGISTER_SEQ(factory, ECMA48_TSS,  kCSI,   Pn, 0, "\x20\x45");
    REGISTER_SEQ(factory, ECMA48_VPA,  kCSI,   Pn, 0, "\x64");
    REGISTER_SEQ(factory, ECMA48_VPB,  kCSI,   Pn, 0, "\x6b");
    REGISTER_SEQ(factory, ECMA48_VPR,  kCSI,   Pn, 0, "\x65");
    REGISTER_SEQ(factory, ECMA48_VT,  "\x0b",  No, 0, "");
    REGISTER_SEQ(factory, ECMA48_VTS,  kESC,   No, 0, "\x4a");
}
