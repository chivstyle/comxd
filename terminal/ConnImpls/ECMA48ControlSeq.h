//
// (c) 2020 chiv
//
#ifndef _comxd_ECMA48ControlSeq_h_
#define _comxd_ECMA48ControlSeq_h_

#include "ControlSeq.h"
#include <algorithm>

// 7-bit VT
// 8.3.1, C0, ACK
// 06, ASCII
// 8.3.2, C0, APC
// "\x5f",
// 8.3.3, BELL, see VT102
// 8.3.5, BS, see VT102
// 8.3.4, C1, BPH
// "\x42",
// 8.3.6 CAN, see VT102
// 8.3.7 CBT, see 1Pn
// 8.3.8 CCH, is used to indicate that both the preceding graphic character in the data stream,
//            including SPACE, and the control function CCH itself are to be ignored for futher
//            interpretation of the data stream.
// "\x54",
// 8.3.9 CHA, causes the active presentation position to be moved to character position n in the active line
//            in the presentation component, where n equas the value of Pn
//            see 1Pn
// 8.3.10 CHT, causes the active presentation position to be moved to the character position corresponding to
//             the n-th following character tabulation stop in the presentation component, according to the
//             character path, where n equals the value of Pn
// 8.3.11 CMD
// "\x64",
// 8.3.12 CNL, cause the active presentation position to be moved to the first character position of the n-th
//             following line in the presentation component, where n equals the value of Pn
// 8.3.13 CPL
// 8.3.14 CPR
// 8.3.15 CR, see VT102
// 8.3.16 CSI, it's ASCII char '[', tell us the successive seq is control function
// 8.3.17 CTC, see 1Ps
// 8.3.18 CUB, see VT102
// 8.3.19 CUD, see above
// 8.3.20 CUF, see above
// 8.3.21 CUF, see above
// 8.3.22 CUU, see above
// 8.3.23 CVT, see 1Pn, causes the active presentation position to be moved to the corresponding character position
//             of the line corresponding to the n-th following line tabulation stop in the presentation component,
//             where n equals the value of Pn
// 8.3.24 DA, device attributes, see 1Ps
// 8.3.25 DAQ, see 1Ps
// 8.3.26 DCH, see 1Pn
// 8.3.27 DCS
// "\x50",
// 8.3.28 DC1, ASCII 0x11
// 8.3.29 DC2, ASCII 0x12
// 8.3.30 DC3, ASCII 0x13
// 8.3.31 DC4, ASCII 0x14
// 8.3.32 DL, see 1Pn
// 8.3.33 DLE, is used exclusively to provide supplementary transmission control functions. The use of DLE is defined in ISO 1745.
//             ASCII 0x10
// 8.3.34 DMI, causes the manual input facilities of a device to be disabled.
// "\x60",
// 8.3.35 DSR, see 1Ps
// 8.3.36 DTA, see 2Pn
// 8.3.37 EA, see 1Ps
// 8.3.38 ECH, see 1Pn
// 8.3.39 ED, see 1Ps
// 8.3.40 EF, see 1Ps
// 8.3.41 EL, see 1Ps
// 8.3.42 EM, ASCII 0x19
// 8.3.43 EMI is used to enable the manual input facilities of a device
// "\x62",
// 8.3.44 ENQ, ASCII 0x05
// 8.3.45 EOT, ASCII 0x04
// 8.3.46 EPA
// "\x57",
// 8.3.47 ESA
// "\x47",
// 8.3.48 ESC, ASCII 0x1b
// 8.3.49 ETB, ASCII 0x17, ISO 1745
// 8.3.50 ETX, ASCII 0x03
// 8.3.51 FF, ASCII 0x0c, causes the active presentation position to be moved to the corresponding character position of
//            the line at the page home position of the next form or page in the presentation component. The page home
//            position is established by the parameter value of SET PAGE HOME (SPH).
// 8.3.52 FNK, is a control function in which the parameter value identifies the function key which has been operated.
//             CSI Pn 02/00 05/07, 1Pn_with_1_suffix_blank
// 8.3.53 FNT, CSI Ps1;Ps2 02/00 04/04, see 2Ps_with_1_suffix_blank
// 8.3.54 GCC, CSI Ps 02/00 05/15, see 1Ps_with_1_suffix_blank
// 8.3.55 GSM, CSI Pn1;Pn2 02/00 04/02 see 2Pn_with_1_suffix_blank
// 8.3.56 GSS, see 1Pn_with_1_suffix_blank
// 8.3.57 HPA, see 1Pn
// 8.3.58 HPB, see 1Pn
// 8.3.59 HPR, see 1Pn
// 8.3.60 HT, ASCII 0x09, TAB
// 8.3.61 HTJ, causes the contents of the active field(the field in the presentation component that contains the active presentation position)
//             to be shifted forward so that it ends at the character position preceding the following character tabulation stop. The active
//             presentation position is moved to that following character tabulation stop. The character position which precede the beginning
//             of the shifted string are put into the erased state.
// "\x49",
// 8.3.62 HTS causes a character tabulation stop to be set at the active presentation position in the presentation component.
//        The number of lines affected depends on the setting of the TABULATION STOP MODE (TSM).
// "\x48",
// 8.3.63 HVP, see 2Pn
// 8.3.64 ICH, see 1Pn
// 8.3.65 IDCS, see 1Ps_with_1_suffix_blank
// 8.3.66 IGS, see 1Ps_with_1_suffix_blank
//             IGS is used to indicate that a repertoire of the graphic characters of
//             ISO/IEC 10367 is used in the subsequent text.
//             The parameter value of IGS identifies a graphic character repertoire
//             registered in accordance with ISO/IEC 7350
// 8.3.67 IL, see 1Pn
// 8.3.68 INT, is used to indicate to the receiving device that the current process is to be interrupted and
//             agreed procedure is to be initiated. This control function is applicable to either direction
//             of transmission.
// "\x61",
// 8.3.69 ASCII, 0x1f
// 8.3.70 ASCII, 0x1e
// 8.3.71 ASCII, 0x1d
// 8.3.72 ASCII, 0x1c
// 8.3.73 JFY, see vPs_with_1_suffix_blank
// 8.3.74 ASCII, 0x1a
// 8.3.75 LS0, ASCII 0x0f
// 8.3.76 LS1, ASCII 0x0e, used in 8-bit environments only.
// 8.3.77 LS1R, is used for code extension purposes. It causes the meanings of the bit combinations following it
//              int the data stream to be changed
//              The use of LS1R is defined in Standard ECMA-35
// "\x7e",
// 8.3.78 LS2
// "\x6e"
// 8.3.79 LS2R
// "\x7d",
// // 8.3.80 LS3
// "\x6f",
// 8.3.81 LS3R
// "\x7c",
// 8.3.82 see 1Ps
// 8.3.83 MW
// "\x55",
// 8.3.84 NAK, ASCII, 0x15
// 8.3.85 NBH, is used to indicate a point where a line break shall not occur when text is formatted.
//        NBH may occur between two graphic characters either or both of which may be SPACE.
// "\x43",
// 8.3.86 NEL,
// "\x45",
// 8.3.87 NP, see 1Pn
// 8.3.88 NUL, ASCII 0x0
// 8.3.89 OSC
// "\x5d",
// 8.3.90 PEC, see 1Ps_with_1_suffix_blank
// 8.3.91 PFS, see 1Ps_with_1_suffix_blank
// 8.3.92 PLD
// "\x4b",
// 8.3.93 PLU
// "\x4c",
// 8.3.94 PM
// "\x5e",
// 8.3.95 PP, see 1Pn
// 8.3.96 PPA, see 1Pn_with_1_suffix_blank
// 8.3.97 PPB, see 1Pn_with_1_suffix_blank
// 8.3.98 PPR, see 1Pn_with_1_suffix_blank
// 8.3.99 PTX, see 1Ps
// 8.3.100 PU1
// "\x51",
// 8.3.101 PU2
// "\x52",
// 8.3.102 QUAD, see vPs_with_1_suffix_blank
// 8.3.103 REP, see 1Pn
// 8.3.104 RI
// "\x4d",
// 8.3.105 RIS
// "\x63",
// 8.3.106 RM, see vPs
// 8.3.107 SACS, see 1Pn_with_1_suffix_blank
// 8.3.108 SAPV, see vPs_with_1_suffix_blank
// 8.3.109 SCI
// "\x5a",
// 8.3.110 SCO, see 2Ps_with_1_suffix_blank
// 8.3.111 SCP, see 2Ps_with_1_suffix_blank
// 8.3.112 SCS, see 1Pn_with_1_suffix_blank
// 8.3.113 SD, see 1Pn
// 8.3.114 SDS, see 1Ps
// 8.3.115 SEE, see 1Ps
// 8.3.116 SEF, see 2Ps_with_1_suffix_blank
// 8.3.117 SGR, see vPs
// 8.3.118 SHS, see 1Ps_with_1_suffix_blank
// 8.3.119 SI, ASCII 0x0f
// 8.3.120 SIMD, see 1Ps
// 8.3.121 SL, see 1Pn_with_1_suffix_blank
// 8.3.122 SLH, see 1Pn_with_1_suffix_blank
// 8.3.123 SLL, see 1Pn_with_1_suffix_blank
// 8.3.124 SLS, see 1Pn_with_1_suffix_blank
// 8.3.125 SM, see vPs
// 8.3.126 SO, ASCII 0x0e
// 8.3.127 SOH, ASCII 0x01
// 8.3.128 SOS
// "\x58",
// 8.3.129 SPA
// "\x56",
// 8.3.130 SPD, see 2Ps_with_1_suffix_blank
// 8.3.131 SPH, see 1Pn_with_1_suffix_blank
// 8.3.132 SPI, see 2Pn_with_1_suffix_blank
// 8.3.133 SPL, see 1Pn_with_1_suffix_blank
// 8.3.134 SPQR, see 1Ps_with_1_suffix_blank
// 8.3.135 SR, see 1Pn_with_1_suffix_blank
// 8.3.136 SRCS, see 1Pn_with_1_suffix_blank
// 8.3.137 SRS, see 1Ps
// 8.3.138 SSA
// "\x46",
// 8.3.139 SSU, see 1Ps_with_1_suffix_blank
// 8.3.140 SSW, see 1Pn_with_1_suffix_blank
// 8.3.141 SS2
// "\x4e",
// 8.3.142 SS3
// "\x4f",
// 8.3.143 ST
// "\x5c",
// 8.3.144 STAB, see 1Ps_with_1_suffix_blank
// 8.3.145 STS
// "\x53",
// 8.3.146 STX, ASCII 0x02
// 8.3.147 SU, see 1Pn
// 8.3.148 SUB, ASCII 0x1a
// 8.3.149 SVS, see 1Ps_with_1_suffix_blank
// 8.3.150 SYN, ASCII 0x16
// 8.3.151 TAC, see 1Pn_with_1_suffix_blank
// 8.3.152 TALE, see 1Pn_with_1_suffix_blank
// 8.3.153 TATE, see 1Pn_with_1_suffix_blank
// 8.3.154 TBC, see 1Ps
// 8.3.155 TCC, see 2Pn_with_1_suffix_blank
// 8.3.156 TSR, see 1Pn_with_1_suffix_blank
// 8.3.157 TSS, see 1Pn_with_1_suffix_blank
// 8.3.158 VPA, see 1Pn
// 8.3.159 VPB, see 1Pn
// 8.3.160 VPR, see 1Pn
// 8.3.161 VT, ASCII 0x0b
// 8.3.162 VTS
// "\x4a"
// C1 set
static const char kECMA48_C1[] = {
    0x42,
    0x43,
    0x45,
    0x46,
    0x47,
    0x48,
    0x49,
    0x4a,
    0x4b,
    0x4c,
    0x4d,
    0x4e,
    0x4f,
    0x50,
    0x51,
    0x52,
    0x53,
    0x54,
    0x55,
    0x56,
    0x57,
    0x58,
    0x5a,
    0x5c,
    0x5d,
    0x5e,
    0x5f,
    0x60,
    0x61,
    0x62,
    0x63,
    0x64,
    0x6e,
    0x6f,
    0x7c,
    0x7d,
    0x7e
};

static const char* kECMA48CtrlSeqs[] = {
    "[H",
    "[m"
};

enum ECMA48FuncionType {
    ECMA48_Trivial = ECMA48_SEQ_BEGIN,
    //-----------------------------------------------------------------------------
    ECMA48_CBT, // 1Pn5a
    ECMA48_CHA, // 1Pn47
    ECMA48_CHT, // 1Pn49
    ECMA48_CNL, // 1Pn45
    ECMA48_CPL, // 1Pn46
    ECMA48_CUB, // 1Pn44
    ECMA48_CUD, // 1Pn42
    ECMA48_CUF, // 1Pn43
    ECMA48_CUU, // 1Pn41
    ECMA48_CVT, // 1Pn59
    ECMA48_DCH, // 1Pn50
    ECMA48_DL,  // 1Pn4d
    ECMA48_ECH, // 1Pn58
    ECMA48_ICH, // 1Pn40
    ECMA48_NP,   // 1Pn55
    ECMA48_IL,   // 1Pn4c
    ECMA48_PP,   // 1Pn56
    ECMA48_REP,  // 1Pn62
    ECMA48_VPA,  // 1Pn64
    ECMA48_VPB,  // 1Pn6b
    ECMA48_VPR,  // 1Pn65
    ECMA48_SD,   // 1Pn54
    ECMA48_SU,   // 1Pn53
    ECMA48_HPA,  // 1Pn60
    ECMA48_HPB,  // 1Pn6a
    ECMA48_HPR,  // 1PB61
    //-----------------------------------------------------------------------------
    ECMA48_FNK, // 1PnB57
    ECMA48_GSS, // 1PnB43
    ECMA48_PPA, // 1PnB50
    ECMA48_PPB, // 1PnB52
    ECMA48_PPR, // 1PnB51
    ECMA48_SACS, // 1PnB5c
    ECMA48_SCS,  // 1PnB67
    ECMA48_SL,   // 1PnB40
    ECMA48_SLH,  // 1PnB55
    ECMA48_SLL,  // 1PnB56
    ECMA48_SLS,  // 1PnB68
    ECMA48_SPL,  // 1PnB6a
    ECMA48_SR,   // 1PnB41
    ECMA48_SRCS, // 1PnB66
    ECMA48_TSR,  // 1PnB64
    ECMA48_TSS,  // 1PnB45
    ECMA48_TALE, // 1PnB61
    ECMA48_TATE, // 1PnB60
    ECMA48_SSW,  // 1PnB5b
    ECMA48_TAC,  // 1PnB62
    ECMA48_SPH,  // 1PnB69
    //-----------------------------------------------------------------------------
    ECMA48_CPR,  // 2Pn52
    ECMA48_HVP,  // 2Pn66
    ECMA48_CUP,  // 1Pn48
    //-----------------------------------------------------------------------------
    ECMA48_DTA, // 2PnB54
    ECMA48_GSM, // 2PnB42
    ECMA48_SPI, // 2PnB47
    ECMA48_TCC, // 2PnB63
    //-----------------------------------------------------------------------------
    ECMA48_DA,  // 1Ps63
    ECMA48_DSR, // 1Ps6e
    ECMA48_EA,  // 1Ps4f
    ECMA48_ED,  // 1Ps4a
    ECMA48_EF,  // 1Ps4e
    ECMA48_EL,  // 1Ps4b
    ECMA48_MC,  // 1Ps69
    ECMA48_PTX, // 1Ps5c
    ECMA48_SRS, // 1Ps5b
    ECMA48_TBC, // 1Ps67
    ECMA48_SDS,  // 1Ps5d
    ECMA48_SEE,  // 1Ps51
    ECMA48_SIMD, // 1Ps5e
    //-----------------------------------------------------------------------------
    ECMA48_GCC,  // 1PsB5f
    ECMA48_IDCS, // 1PsB4f
    ECMA48_IGS,  // 1PsB4d
    ECMA48_PEC,  // 1PsB5a
    ECMA48_PFS,  // 1PsB4a
    ECMA48_SCO,  // 1PsB65
    ECMA48_SPQR, // 1PsB58
    ECMA48_SSU,  // 1PsB49
    ECMA48_STAB, // 1PsB5e
    ECMA48_SVS,  // 1PsB4c
    ECMA48_SHS,  // 1PsB4b
    //-----------------------------------------------------------------------------
    ECMA48_FNT, // 2PsB44
    ECMA48_SCP, // 2PsB6b
    ECMA48_SEF, // 2PsB59
    ECMA48_SPD, // 2PsB53
    //-----------------------------------------------------------------------------
    ECMA48_CTC, // vPs57
    ECMA48_DAQ, // vPs6f
    ECMA48_RM,  // vPs6c
    ECMA48_SM,  // vPs68
    ECMA48_SGR, // vPs6d
    //-----------------------------------------------------------------------------
    ECMA48_JFY,  // vPsB46
    ECMA48_QUAD, // vPsB48
    ECMA48_SAPV, // vPsB5d
    //-----------------------------------------------------------------------------
    ECMA48_C1,
    //
    ECMA48_SeqType_Endup
};
static_assert(ECMA48_SeqType_Endup < ECMA48_SEQ_END, "ECMA48_SeqType_Endup should be less than ECMA48_SEQ_END");

static inline int IsECMA48_C1(const std::string& seq)
{
    if (seq.size() == 1) {
        if (std::binary_search(&kECMA48_C1[0], &kECMA48_C1[sizeof(kECMA48_C1)]/* invalid for end*/, seq[0]))
            return ECMA48_C1;
    }
    return SEQ_NONE;
}

static inline int IsECMA48_vPs(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[')
        return SEQ_PENDING; // pending
    else {
        if (seq[0] == '[') {
            size_t b = 1;
            while (b < seq_sz) {
                if (seq[b] >= '0' && seq[b] <= '9' || seq[b] == ';')
                    b++;
                else
                    break;
            }
            if (b == seq_sz)
                return SEQ_PENDING;
            else {
                switch (seq[b]) {
                case 0x57: return ECMA48_CTC; // vPs57
                case 0x6f: return ECMA48_DAQ; // vPs6f
                case 0x6c: return ECMA48_RM;  // vPs6c
                case 0x68: return ECMA48_SM;  // vPs68
                case 0x6d: return ECMA48_SGR; // vPs6d
                }
            }
        }
    }
    return SEQ_NONE;
}

static inline int IsECMA48_vPsB(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[')
        return SEQ_PENDING; // pending
    else {
        if (seq[0] == '[') {
            size_t b = 1;
            while (b < seq_sz) {
                if (seq[b] >= '0' && seq[b] <= '9' || seq[b] == ';')
                    b++;
                else
                    break;
            }
            if (b == seq_sz)
                return SEQ_PENDING;
            else {
                if (seq_sz - b == 1 && seq[b] == 0x20) return SEQ_PENDING;
                else if (seq_sz - b == 2 && seq[b] == 0x20) {
                    switch (seq[b+1]) {
                    case 0x46: return ECMA48_JFY;  // vPsB46
                    case 0x48: return ECMA48_QUAD; // vPsB48
                    case 0x5d: return ECMA48_SAPV; // vPsB5d
                    }
                }
            }
        }
    }
    return SEQ_NONE;
}

static inline int IsECMA48_2PsB(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[')
        return SEQ_PENDING; // pending
    else {
        if (seq[0] == '[') {
            size_t b = 1;
            while (b < seq_sz) {
                if (seq[b] >= '0' && seq[b] <= '9')
                    b++;
                else
                    break;
            }
            if (b == seq_sz)
                return SEQ_PENDING;
            else {
                if (seq[b] == ';') {
                    b++; // skip ;
                    while (b < seq_sz) {
                        if ((seq[b] >= '0' && seq[b] <= '9'))
                            b++;
                        else
                            break;
                    }
                    if (b == seq_sz)
                        return SEQ_PENDING;
                    else {
                        if (seq_sz - b == 1 && seq[b] == 0x20) return SEQ_PENDING;
                        else if (seq_sz - b == 2 && seq[b] == 0x20) {
                            switch (seq[b+1]) {
                            case 0x44: return ECMA48_FNT; // 2PsB44
                            case 0x6b: return ECMA48_SCP; // 2PsB6b
                            case 0x59: return ECMA48_SEF; // 2PsB59
                            case 0x53: return ECMA48_SPD; // 2PsB53
                            }
                        }
                    }
                } else
                    return SEQ_NONE;
            }
        }
    }
    return SEQ_NONE;
}

static inline int IsECMA48_2PnB(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[')
        return SEQ_PENDING; // pending
    else {
        if (seq[0] == '[') {
            size_t b = 1;
            while (b < seq_sz) {
                if (seq[b] >= '0' && seq[b] <= '9')
                    b++;
                else
                    break;
            }
            if (b == seq_sz)
                return SEQ_PENDING;
            else {
                if (seq[b] == ';') {
                    b++; // skip ;
                    while (b < seq_sz) {
                        if ((seq[b] >= '0' && seq[b] <= '9'))
                            b++;
                        else
                            break;
                    }
                    if (b == seq_sz)
                        return SEQ_PENDING;
                    else {
                        if (seq_sz - b == 1 && seq[b] == 0x20) return SEQ_PENDING;
                        else if (seq_sz - b == 2 && seq[b] == 0x20) {
                            switch (seq[b+1]) {
                            case 0x54: return ECMA48_DTA; // 2PnB54
                            case 0x42: return ECMA48_GSM; // 2PnB42
                            case 0x47: return ECMA48_SPI; // 2PnB47
                            case 0x63: return ECMA48_TCC; // 2PnB63
                            }
                        }
                    }
                } else
                    return SEQ_NONE;
            }
        }
    }
    return SEQ_NONE;
}

static inline int IsECMA48_2Pn(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[')
        return SEQ_PENDING; // pending
    else {
        if (seq[0] == '[') {
            size_t b = 1;
            while (b < seq_sz) {
                if (seq[b] >= '0' && seq[b] <= '9')
                    b++;
                else
                    break;
            }
            if (b == seq_sz)
                return SEQ_PENDING;
            else {
                if (seq[b] == ';') {
                    b++; // skip ;
                    while (b < seq_sz) {
                        if ((seq[b] >= '0' && seq[b] <= '9'))
                            b++;
                        else
                            break;
                    }
                    if (b == seq_sz)
                        return SEQ_PENDING;
                    else {
                        switch (seq[b]) {
                        case 0x52: return ECMA48_CPR;  // 2Pn52
                        case 0x66: return ECMA48_HVP;  // 2Pn66
                        case 0x48: return ECMA48_CUP; //  2Pn48
                        }
                    }
                } else
                    return SEQ_NONE;
            }
        }
    }
    return SEQ_NONE;
}
//
static inline int IsECMA48_1PsB(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[')
        return SEQ_PENDING; // pending
    else {
        if (seq[0] == '[') {
            size_t b = 1;
            while (b < seq_sz) {
                if (seq[b] >= '0' && seq[b] <= '9')
                    b++;
                else
                    break;
            }
            if (b == seq_sz)
                return SEQ_PENDING;
            else {
                if (seq_sz - b == 1 && seq[b] == 0x20) return SEQ_PENDING;
                else if (seq_sz - b == 2 && seq[b] == 0x20) {
                    switch (seq[b+1]) {
                    case 0x5f: return ECMA48_GCC;  // 1PsB5f
                    case 0x4f: return ECMA48_IDCS; // 1PsB4f
                    case 0x4d: return ECMA48_IGS;  // 1PsB4d
                    case 0x5a: return ECMA48_PEC;  // 1PsB5a
                    case 0x4a: return ECMA48_PFS;  // 1PsB4a
                    case 0x65: return ECMA48_SCO;  // 1PsB65
                    case 0x58: return ECMA48_SPQR; // 1PsB58
                    case 0x49: return ECMA48_SSU;  // 1PsB49
                    case 0x5e: return ECMA48_STAB; // 1PsB5e
                    case 0x4c: return ECMA48_SVS;  // 1PsB4c
                    case 0x4b: return ECMA48_SHS;  // 1PsB4b
                    }
                }
            }
        }
    }
    return SEQ_NONE;
}
static inline int IsECMA48_2Ps(const std::string& seq)
{
    return SEQ_NONE;
}
static inline int IsECMA48_1Ps(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[')
        return SEQ_PENDING; // pending
    else {
        if (seq[0] == '[') {
            size_t b = 1;
            while (b < seq_sz) {
                if (seq[b] >= '0' && seq[b] <= '9')
                    b++;
                else
                    break;
            }
            if (b == seq_sz)
                return SEQ_PENDING;
            else {
                switch (seq[b]) {
                case 0x63: return ECMA48_DA;  // 1Ps63
                case 0x6e: return ECMA48_DSR; // 1Ps6e
                case 0x4f: return ECMA48_EA;  // 1Ps4f
                case 0x4a: return ECMA48_ED;  // 1Ps4a
                case 0x4e: return ECMA48_EF;  // 1Ps4e
                case 0x4b: return ECMA48_EL;  // 1Ps4b
                case 0x69: return ECMA48_MC;  // 1Ps69
                case 0x5c: return ECMA48_PTX; // 1Ps5c
                case 0x5b: return ECMA48_SRS; // 1Ps5b
                case 0x67: return ECMA48_TBC; // 1Ps67
                case 0x5d: return ECMA48_SDS;  // 1Ps5d
                case 0x51: return ECMA48_SEE;  // 1Ps51
                case 0x5e: return ECMA48_SIMD; // 1Ps5e
                }
            }
        }
    }
    return SEQ_NONE;
}

static inline int IsECMA48_1PnB(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[')
        return SEQ_PENDING; // pending
    else {
        if (seq[0] == '[') {
            size_t b = 1;
            while (b < seq_sz) {
                if (seq[b] >= '0' && seq[b] <= '9')
                    b++;
                else
                    break;
            }
            if (b == seq_sz)
                return SEQ_PENDING;
            else {
                if (seq_sz - b == 1 && seq[b] == 0x20) return SEQ_PENDING;
                else if (seq_sz - b == 2) {
                    switch (seq[b+1]) {
                    case 0x57: return ECMA48_FNK; // 1PnB57
                    case 0x43: return ECMA48_GSS; // 1PnB43
                    case 0x50: return ECMA48_PPA; // 1PnB50
                    case 0x52: return ECMA48_PPB; // 1PnB52
                    case 0x51: return ECMA48_PPR; // 1PnB51
                    case 0x5c: return ECMA48_SACS; // 1PnB5c
                    case 0x67: return ECMA48_SCS;  // 1PnB67
                    case 0x40: return ECMA48_SL;   // 1PnB40
                    case 0x55: return ECMA48_SLH;  // 1PnB55
                    case 0x56: return ECMA48_SLL;  // 1PnB56
                    case 0x68: return ECMA48_SLS;  // 1PnB68
                    case 0x6a: return ECMA48_SPL;  // 1PnB6a
                    case 0x41: return ECMA48_SR;   // 1PnB41
                    case 0x66: return ECMA48_SRCS; // 1PnB66
                    case 0x64: return ECMA48_TSR;  // 1PnB64
                    case 0x45: return ECMA48_TSS;  // 1PnB45
                    case 0x61: return ECMA48_TALE; // 1PnB61
                    case 0x60: return ECMA48_TATE; // 1PnB60
                    case 0x5b: return ECMA48_SSW;  // 1PnB5b
                    case 0x62: return ECMA48_TAC;  // 1PnB62
                    case 0x69: return ECMA48_SPH;  // 1PnB69
                    }
                }
            }
        }
    }
    return SEQ_NONE;
}

static inline int IsECMA48_1Pn(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[')
        return SEQ_PENDING; // pending
    else {
        if (seq[0] == '[') {
            size_t b = 1;
            while (b < seq_sz) {
                if (seq[b] >= '0' && seq[b] <= '9')
                    b++;
                else
                    break;
            }
            if (b == seq_sz)
                return SEQ_PENDING;
            else {
                switch (seq[b]) {
                    case 0x5a: return ECMA48_CBT; // 1Pn5a
                    case 0x47: return ECMA48_CHA; // 1Pn47
                    case 0x49: return ECMA48_CHT; // 1Pn49
                    case 0x45: return ECMA48_CNL; // 1Pn45
                    case 0x46: return ECMA48_CPL; // 1Pn46
                    case 0x44: return ECMA48_CUB; // 1Pn44
                    case 0x42: return ECMA48_CUD; // 1Pn42
                    case 0x43: return ECMA48_CUF; // 1Pn43
                    case 0x41: return ECMA48_CUU; // 1Pn41
                    case 0x59: return ECMA48_CVT; // 1Pn59
                    case 0x50: return ECMA48_DCH; // 1Pn50
                    case 0x4d: return ECMA48_DL;  // 1Pn4d
                    case 0x58: return ECMA48_ECH; // 1Pn58
                    case 0x40: return ECMA48_ICH; // 1Pn40
                    case 0x55: return ECMA48_NP;  // 1Pn55
                    case 0x4c: return ECMA48_IL;  // 1Pn4c
                    case 0x56: return ECMA48_PP;  // 1Pn56
                    case 0x62: return ECMA48_REP; // 1Pn62
                    case 0x64: return ECMA48_VPA; // 1Pn64
                    case 0x6b: return ECMA48_VPB; // 1Pn6b
                    case 0x65: return ECMA48_VPR; // 1Pn65
                    case 0x54: return ECMA48_SD;  // 1Pn54
                    case 0x53: return ECMA48_SU;  // 1Pn53
                    case 0x60: return ECMA48_HPA; // 1Pn60
                    case 0x6a: return ECMA48_HPB; // 1PB6a
                    case 0x61: return ECMA48_HPR; // 1PB61
                }
            }
        }
    }
    return SEQ_NONE;
}

static inline int IsECMA48TrivialSeq(const std::string& seq)
{
    size_t vsz = sizeof(kECMA48CtrlSeqs) / sizeof(kECMA48CtrlSeqs[0]);
    size_t seq_sz = seq.length();
    for (size_t k = 0; k < vsz; ++k) {
        size_t pat_sz = strlen(kECMA48CtrlSeqs[k]);
        if (seq_sz > pat_sz)
            continue; // try next
        if (strncmp(kECMA48CtrlSeqs[k], seq.c_str(), seq_sz) == 0) {
            if (seq_sz < pat_sz)
                return SEQ_PENDING; // maybe, you should give more chars
            else if (seq_sz == pat_sz)
                return ECMA48_Trivial; // It's a ECMA48 control seq
        }
    }
    return SEQ_NONE;
}
// match the control seq
// return SEQ_NONE - It's not a ECMA48 control seq absolutely
//        1 - maybe, your should give more characters
//        2 - It's a ECMA48 control seq.
static inline int IsECMA48ControlSeq(const std::string& seq)
{
    std::function<int(const std::string&)> funcs[] = {
        IsECMA48_C1,
        IsECMA48TrivialSeq,
        IsECMA48_1Pn,
        IsECMA48_1PnB,
        IsECMA48_2Pn,
        IsECMA48_2PnB,
        IsECMA48_1Ps,
        IsECMA48_1PsB,
        IsECMA48_2Ps,
        IsECMA48_2PsB,
        IsECMA48_vPs,
        IsECMA48_vPsB
    };
    int ret = SEQ_NONE;
    for (int i = 0; i < sizeof(funcs) / sizeof(funcs[0]); ++i) {
        ret = funcs[i](seq);
        if (ret == SEQ_NONE)
            continue;
        else
            break;
    }
    return ret;
}

#endif
