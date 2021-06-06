//
// (c) 2020 chiv
//
// ANSI X3.64
//
#pragma once

#include "connimpls/ControlSeq.h"
#include <string>
//
enum Ecma48SeqType {
    // C0
    ECMA48_NUL = ECMA48_SEQ_BEGIN,
    ECMA48_SOH,
    ECMA48_STX,
    ECMA48_ETX,
    ECMA48_EOT,
    ECMA48_ENQ,
    ECMA48_ACK,
    ECMA48_BEL,
    ECMA48_BS,
    ECMA48_HT,
    ECMA48_LF,
    ECMA48_VT,
    ECMA48_FF,
    ECMA48_CR,
    ECMA48_SO, // called LS0 in 8bit environment
    ECMA48_SI, // called LS1 in 8bit environment
    ECMA48_DLE,
    ECMA48_DC1,
    ECMA48_DC2,
    ECMA48_DC3,
    ECMA48_DC4,
    ECMA48_NAK,
    ECMA48_SYN,
    ECMA48_ETB,
    ECMA48_CAN,
    ECMA48_EM,
    ECMA48_SUB,
    ECMA48_ESC,
    ECMA48_FS,
    ECMA48_GS,
    ECMA48_RS,
    ECMA48_US,
    ECMA48_DEL,
    // C1
    ECMA48_APC,
    ECMA48_BPH,
    ECMA48_CBT,
    ECMA48_CCH,
    ECMA48_CHA,
    ECMA48_CHT,
    ECMA48_CMD,
    ECMA48_CNL,
    ECMA48_CPL,
    ECMA48_CPR,
    ECMA48_CTC,
    ECMA48_CUB,
    ECMA48_CUD,
    ECMA48_CUF,
    ECMA48_CUP,
    ECMA48_CUU,
    ECMA48_CVT,
    ECMA48_DA,
    ECMA48_DAQ,
    ECMA48_DCH,
    ECMA48_DCS,
    ECMA48_DL,
    ECMA48_DMI,
    ECMA48_DSR,
    ECMA48_DTA,
    ECMA48_EA,
    ECMA48_ECH,
    ECMA48_ED,
    ECMA48_EF,
    ECMA48_EL,
    ECMA48_EMI,
    ECMA48_EPA,
    ECMA48_ESA,
    ECMA48_FNK,
    ECMA48_FNT,
    ECMA48_GCC,
    ECMA48_GSM,
    ECMA48_GSS,
    ECMA48_HPA,
    ECMA48_HPB,
    ECMA48_HPR,
    ECMA48_HTJ,
    ECMA48_HTS,
    ECMA48_HVP,
    ECMA48_ICH,
    ECMA48_IDCS,
    ECMA48_IGS,
    ECMA48_IL,
    ECMA48_INT,
    ECMA48_JFY,
    ECMA48_LS1R,
    ECMA48_LS2,
    ECMA48_LS2R,
    ECMA48_LS3,
    ECMA48_LS3R,
    ECMA48_MC,
    ECMA48_MW,
    ECMA48_NBH,
    ECMA48_NEL,
    ECMA48_NP,
    ECMA48_OSC,
    ECMA48_PEC,
    ECMA48_PFS,
    ECMA48_PLD,
    ECMA48_PLU,
    ECMA48_PM,
    ECMA48_PP,
    ECMA48_PPA,
    ECMA48_PPB,
    ECMA48_PPR,
    ECMA48_PTX,
    ECMA48_PU1,
    ECMA48_PU2,
    ECMA48_QUAD,
    ECMA48_REP,
    ECMA48_RI,
    ECMA48_RIS,
    ECMA48_RM,
    ECMA48_SACS,
    ECMA48_SAPV,
    ECMA48_SCI,
    ECMA48_SCO,
    ECMA48_SCP,
    ECMA48_SCS,
    ECMA48_SD,
    ECMA48_SDS,
    ECMA48_SEE,
    ECMA48_SEF,
    ECMA48_SGR,
    ECMA48_SHS,
    ECMA48_SIMD,
    ECMA48_SL,
    ECMA48_SLH,
    ECMA48_SLL,
    ECMA48_SLS,
    ECMA48_SM,
    ECMA48_SOS,
    ECMA48_SPA,
    ECMA48_SPD,
    ECMA48_SPH,
    ECMA48_SPI,
    ECMA48_SPL,
    ECMA48_SPQR,
    ECMA48_SR,
    ECMA48_SRCS,
    ECMA48_SRS,
    ECMA48_SSA,
    ECMA48_SSU,
    ECMA48_SSW,
    ECMA48_SS2,
    ECMA48_SS3,
    ECMA48_ST,
    ECMA48_STAB,
    ECMA48_STS,
    ECMA48_SU,
    ECMA48_SVS,
    ECMA48_TAC,
    ECMA48_TALE,
    ECMA48_TATE,
    ECMA48_TBC,
    ECMA48_TCC,
    ECMA48_TSR,
    ECMA48_TSS,
    ECMA48_VPA,
    ECMA48_VPB,
    ECMA48_VPR,
    ECMA48_VTS,
    
    ECMA48_SEQ_MAX
};
static_assert(ECMA48_SEQ_MAX < ECMA48_SEQ_END, "You should define ECMA48_SEQ_MAX_COUNT big enough");

void AddEcma48ControlSeqs(ControlSeqFactory* factory);