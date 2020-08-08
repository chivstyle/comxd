//
// (c) 2020 chiv
//
#ifndef _comxd_ECMA48ControlSeq_h_
#define _comxd_ECMA48ControlSeq_h_
// 7-bit VT, so the control seq begins with 0x1b
static const char* kECMA48CtrlSeqs[] = {
    // 8.3.1, C0, ACK
    "\x06",
    // 8.3.2, C0
};

enum ECMA48SeqType {
    ECMA48_ACK,
    ECMA48_APC,
    ECMA48_BEL,
    ECMA48_BPH,
    ECMA48_BS,
    ECMA48_CAN,
    ECMA48_CBT,
    ECMA48_CHA,
    ECMA48_CHT,
    ECMA48_CMD,
    ECMA48_CNL,
    ECMA48_CPL,
    ECMA48_CPR,
    ECMA48_CR,
    ECMA48_CSI,
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
    ECMA48_DC1,
    ECMA48_DC2,
    ECMA48_DC3,
    ECMA48_DC4,
    ECMA48_DL,
    ECMA48_DLE,
    ECMA48_DMI,
    ECMA48_DSR,
    ECMA48_DTA,
    ECMA48_EA,
    ECMA48_ECH,
    ECMA48_ED,
    ECMA48_EF,
    ECMA48_EL,
    ECMA48_EM,
    ECMA48_EMI,
    ECMA48_ENQ,
    ECMA48_EOT,
    ECMA48_EPA,
    ECMA48_ESA,
    ECMA48_ESC,
    ECMA48_ETB,
    ECMA48_ETX,
    ECMA48_FF,
    ECMA48_FNK,
    ECMA48_FNT,
    ECMA48_GCC,
    ECMA48_GSM,
    ECMA48_GSS,
    ECMA48_HPA,
    ECMA48_HPB,
    ECMA48_HPR,
    ECMA48_HT,
    ECMA48_HTJ,
    ECMA48_HTS,
    ECMA48_HVP,
    ECMA48_ICH,
    ECMA48_IDCS,
    ECMA48_IGS,
    ECMA48_IL,
    ECMA48_INT,
    ECMA48_IS1,
    ECMA48_IS2,
    ECMA48_IS3,
    ECMA48_IS4,
    ECMA48_JFY,
    ECMA48_LF,
    ECMA48_LS0,
    ECMA48_LS1,
    ECMA48_LS1R,
    ECMA48_LS2,
    ECMA48_LS2R,
    ECMA48_LS3,
    ECMA48_LS3R,
    ECMA48_MC,
    ECMA48_MW,
    ECMA48_NAK,
    ECMA48_NBH,
    ECMA48_NEL,
    ECMA48_NP,
    ECMA48_NUL,
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
    ECMA48_SI,
    ECMA48_SIMD,
    ECMA48_SL,
    ECMA48_SLH,
    ECMA48_SLL,
    ECMA48_SLS,
    ECMA48_SM,
    ECMA48_SO,
    ECMA48_SOH,
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
    ECMA48_STX,
    ECMA48_SU,
    ECMA48_SUB,
    ECMA48_SVS,
    ECMA48_SYN,
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
    ECMA48_VT,
    ECMA48_VTS
};

static inline int IsECMA48_2Pn(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[') return 1; // pending
    else {
        if (seq[0] == '[') {
            size_t b = 1;
            while (b < seq_sz) {
                if (seq[b] >= '0' && seq[b] <= '9')
                    b++;
                else break;
            }
            if (b == seq_sz) return 1;
            else {
                if (seq[b] == ';') {
                    while (b < seq_sz) {
                        if ((seq[b] >= '0' && seq[b] <= '9') || seq[b] == ' ')
                            b++;
                        else break;
                    }
                    if (b == seq_sz) return 1; else {
                        switch (seq[b]) {
                        case 0x52: return ECMA48_CPR;
                        case 0x54: return ECMA48_DTA;
                        }
                    }
                } else return 0;
            }
        }
    }
    return 0;
}
// TODO: pattern with 1 Ps
static inline int IsECMA48_1Ps(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[') return 1; // pending
    else {
        if (seq[0] == '[') {
            size_t b = 1;
            while (b < seq_sz) {
                if (seq[b] >= '0' && seq[b] <= '9')
                    b++;
                else break;
            }
            if (b == seq_sz) return 1;
            else {
                switch (seq[b]) {
                }
            }
        }
    }
    return 0;
}

static inline int IsECMA48_1Pn(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[') return 1; // pending
    else {
        if (seq[0] == '[') {
            size_t b = 1;
            while (b < seq_sz) {
                if (seq[b] >= '0' && seq[b] <= '9')
                    b++;
                else break;
            }
            if (b == seq_sz) return 1;
            else {
                switch (seq[b]) {
                case 0x5a: return ECMA48_CBT;
                case 0x47: return ECMA48_CHA;
                case 0x49: return ECMA48_CHT;
                case 0x45: return ECMA48_CNL;
                case 0x46: return ECMA48_CPL;
                case 0x57: return ECMA48_CTC;
                case 0x59: return ECMA48_CVT;
                case 0x6f: return ECMA48_DAQ;
                case 0x50: return ECMA48_DCH;
                case 0x4d: return ECMA48_DL;
                case 0x6e: return ECMA48_DSR;
                }
            }
        }
    }
    return 0;
}

static inline int IsECMA48TrivialSeq(const std::string& seq)
{
    size_t vsz = sizeof(kECMA48CtrlSeqs) / sizeof(kECMA48CtrlSeqs[0]);
    size_t seq_sz = seq.length();
    for (size_t k = 0; k < vsz; ++k) {
        size_t pat_sz = strlen(kECMA48CtrlSeqs[k]);
        if (seq_sz > pat_sz) continue; // try next
        if (strncmp(kECMA48CtrlSeqs[k], seq.c_str(), seq_sz) == 0) {
            if (seq_sz < pat_sz) return 1; // maybe, you should give more chars
            else if (seq_sz == pat_sz) return 2; // It's a ECMA48 control seq
        }
    }
    return 0;
}
// match the control seq
// return 0 - It's not a ECMA48 control seq absolutely
//        1 - maybe, your should give more characters
//        2 - It's a ECMA48 control seq.
static inline int IsECMA48ControlSeq(const std::string& seq)
{
    std::function<int(const std::string&)> funcs[] = {
        IsECMA48TrivialSeq
    };
    int ret = 0;
    for (int i = 0; i < sizeof(funcs) / sizeof(funcs[0]); ++i) {
        ret = funcs[i](seq);
        if (ret == 0) continue; else break;
    }
    return ret;
}

#endif
