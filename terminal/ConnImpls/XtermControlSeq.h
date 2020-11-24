//
// (c) 2020 chiv
//
#ifndef _comxd_XtermControlSeq_h_
#define _comxd_XtermControlSeq_h_

#include "ControlSeq.h"

// [http://xfree86.org/4.7.0/ctlseqs.html]
static const char* kXtermCtrlSeqs[] = {
    // save, restore alternative screen
    "[?1049h",
    "[?1049l",
    // controls beginning with ESC
    " F", // 7-bit controls C1
    " G", // 8-bit controls C1
    " L", // Set ANSI conformance level 1, ecma-43
    " M", // Set ANSI conformance level 2, ecma-43
    " N", // Set ANSI conformance level 3, ecma-43
    "#3", // DEC double-height line, top half
    "#4", // DEC double-height line, bottom half
    "#5", // DEC single-width line, VT100
    "#6", // DEC double-width line, VT100
    "#8", // DEC screen alignment test, VT100
    "%@", // Select default character set. That is ISO 8859-1 (ISO 2022).
    "%G", // Select UTF-8 character set, ISO 2022.
    // G0 set, vt100
    "(A", // UK, vt100
    "(B", // US, vt100
    "(C", "(5", // Finnish, vt200
    "(H", "(7", // Wwedish, vt200
    "(K", // German, vt200
    "(Q", "(9", // French Canadian, vt200
    "(R", "(f", // French, vt200
    "(Y", // Italian, vt200
    "(Z", // Spanish, vt200
    "(4", // Dutch, vt200
    "(\">", // Greek, vt500
    "(%2", // Turkish, vt500
    "(%6", // Portuguese, vt300
    "(%=", // Hebrew, vt500
    "(=", // Swiss, vt200
    "(`", "(E", "(6", // Norwegian/Danish, vt200
    "(0", // DEC special character and line drawing set, vt100
    "(<", // DEC supplemental, vt200
    "(>", // DEC technical, vt300
    "(\"4", // DEC Hebrew, vt500
    "(\"?", // DEC Greek, vt500
    "(%0", // DEC Turkish, v500
    "(&4", // DEC Cyrillic, vt500
    "(%3", // SCS NRCS, vt500
    "(&5", // DEC Russian, vt500
    // G1 set, vt100
    ")A", // UK, vt100
    ")B", // US, vt100
    ")C", "(5", // Finnish, vt200
    ")H", "(7", // Wwedish, vt200
    ")K", // German, vt200
    ")Q", "(9", // French Canadian, vt200
    ")R", "(f", // French, vt200
    ")Y", // Italian, vt200
    ")Z", // Spanish, vt200
    ")4", // Dutch, vt200
    ")\">", // Greek, vt500
    ")%2", // Turkish, vt500
    ")%6", // Portuguese, vt300
    ")%=", // Hebrew, vt500
    ")=", // Swiss, vt200
    ")`", "(E", "(6", // Norwegian/Danish, vt200
    ")0", // DEC special character and line drawing set, vt100
    ")<", // DEC supplemental, vt200
    ")>", // DEC technical, vt300
    ")\"4", // DEC Hebrew, vt500
    ")\"?", // DEC Greek, vt500
    ")%0", // DEC Turkish, v500
    ")&4", // DEC Cyrillic, vt500
    ")%3", // SCS NRCS, vt500
    ")&5", // DEC Russian, vt500
    // G2 set, vt220
    "*A", // UK, vt100
    "*B", // US, vt100
    "*C", "(5", // Finnish, vt200
    "*H", "(7", // Wwedish, vt200
    "*K", // German, vt200
    "*Q", "(9", // French Canadian, vt200
    "*R", "(f", // French, vt200
    "*Y", // Italian, vt200
    "*Z", // Spanish, vt200
    "*4", // Dutch, vt200
    "*\">", // Greek, vt500
    "*%2", // Turkish, vt500
    "*%6", // Portuguese, vt300
    "*%=", // Hebrew, vt500
    "*=", // Swiss, vt200
    "*`", "(E", "(6", // Norwegian/Danish, vt200
    "*0", // DEC special character and line drawing set, vt100
    "*<", // DEC supplemental, vt200
    "*>", // DEC technical, vt300
    "*\"4", // DEC Hebrew, vt500
    "*\"?", // DEC Greek, vt500
    "*%0", // DEC Turkish, v500
    "*&4", // DEC Cyrillic, vt500
    "*%3", // SCS NRCS, vt500
    "*&5", // DEC Russian, vt500
    // G3 set, vt220
    "+A", // UK, vt100
    "+B", // US, vt100
    "+C", "(5", // Finnish, vt200
    "+H", "(7", // Wwedish, vt200
    "+K", // German, vt200
    "+Q", "(9", // French Canadian, vt200
    "+R", "(f", // French, vt200
    "+Y", // Italian, vt200
    "+Z", // Spanish, vt200
    "+4", // Dutch, vt200
    "+\">", // Greek, vt500
    "+%2", // Turkish, vt500
    "+%6", // Portuguese, vt300
    "+%=", // Hebrew, vt500
    "+=", // Swiss, vt200
    "+`", "(E", "(6", // Norwegian/Danish, vt200
    "+0", // DEC special character and line drawing set, vt100
    "+<", // DEC supplemental, vt200
    "+>", // DEC technical, vt300
    "+\"4", // DEC Hebrew, vt500
    "+\"?", // DEC Greek, vt500
    "+%0", // DEC Turkish, v500
    "+&4", // DEC Cyrillic, vt500
    "+%3", // SCS NRCS, vt500
    "+&5", // DEC Russian, vt500
    // G1 set, vt300
    "-A", // ISO Latin-1 Supplemental, vt300
    "-F", // ISO Greek Supplemental, vt500
    "-H", // ISO Hebrew Supplemental, vt500
    "-L", // ISO Latin-Cyrillic, vt500
    "-M", // ISO Latin-5 Supplemental, vt500
    // G2 set, vt300
    ".A", // ISO Latin-1 Supplemental, vt300
    ".F", // ISO Greek Supplemental, vt500
    ".H", // ISO Hebrew Supplemental, vt500
    ".L", // ISO Latin-Cyrillic, vt500
    ".M", // ISO Latin-5 Supplemental, vt500
    // G3 set, vt300
    "/A", // ISO Latin-1 Supplemental, vt300
    "/F", // ISO Greek Supplemental, vt500
    "/H", // ISO Hebrew Supplemental, vt500
    "/L", // ISO Latin-Cyrillic, vt500
    "/M", // ISO Latin-5 Supplemental, vt500
    //
    "6", // back index, vt420 and up
    "7", // save cursor, vt100
    "8", // restore cursor, vt100
    "9", // forward index, vt420 and up
    "=", // Application keypad
    ">", // Normal keypad, vt100
    "F", // cursor to lower left corner of screen.This is enabled by the hpLowerleftBugCompat resource
    "c", // Full reset(RIS), vt100
    "l", // Memory lock. Locks memory above the cursor
    "m", // Memory unlock
    "n", // Invoke the G2 Character Set as GL (LS2) as GL.
    "o", // Invoke the G3 Character Set as GL (LS3) as GL.
    "|", // Invoke the G3 Character Set as GR (LS3R).
    "}", // Invoke the G2 Character Set as GR (LS2R).
    "~", // Invoke the G1 Character Set as GR (LS1R), VT100.
    //------------------------------------------------------------------------------------------
    "[?3J", // clear saved lines, xterm
    //------------------------------------------------------------------------------------------
    "[!p", // DECSTR, soft terminal reset
    "[#q"
};

enum XtermSeqType {
    Xterm_Trivial = XTERM_SEQ_BEGIN,
    Xterm_XTPUSHCOLORS,
    Xterm_XTPOPCOLORS,
    Xterm_XTREPORTCOLORS,
    Xterm_XTSMGRAPHICS,
    Xterm_XTHIMOUSE,
    Xterm_XTRMTITLE,
    Xterm_XTMODKEYS,
    Xterm_XTDISABLEMODKEYS,
    Xterm_XTSMPOINTER,
    Xterm_XTPUSHSGR,
    Xterm_XTVERSION,
    Xterm_XTRESTORE,
    Xterm_XTWINOPS,
    Xterm_XTSMTITLE,
    Xterm_XTSAVE,
    Xterm_TertiaryDA,
    Xterm_SecondaryDA,
    Xterm_DECSET,
    Xterm_DECMC,
    Xterm_DECRST,
    Xterm_DECSCL,
    Xterm_DECRQM,  // request ANSI mode
    Xterm_DECRQPM, // request dec private mode
    Xterm_DECLL,
    Xterm_DECSCUSR,
    Xterm_DECSCA,
    Xterm_DECCARA,
    Xterm_DECSLRM,
    Xterm_DECSWBV,
    Xterm_OSC,
    Xterm_SeqType_Endup
};

static_assert(Xterm_SeqType_Endup < XTERM_SEQ_END, "Xterm_SeqType_Endup should be less than XTERM_SEQ_END");
// CSI ? Pi;Pa;Pv S
// Pi 1,2,3
// Pa 1,2,3,4
// Pv if Pa!=3, ignore it
//    1, or more integers
// check strictly
static inline int IsXterm_XTSMGRAPHICS(const std::string& seq)
{
	size_t seq_sz = seq.length();
	if (seq[0] != '[') return SEQ_NONE;
	if (seq_sz < 2) return SEQ_PENDING;
	{
        if (seq[0] == '[' && seq[1] == '?') {
            if (seq_sz < 3) return SEQ_PENDING;
            // Pi
            if (seq[2] < '1' || seq[2] > '3') return SEQ_NONE;
            if (seq_sz < 4) return SEQ_PENDING;
            // ;
            if (seq[3] != ';') return SEQ_NONE;
            if (seq_sz < 5) return SEQ_PENDING;
            // Pa
            if (seq[4] < '1' || seq[4] > '4') return SEQ_NONE;
            if (seq_sz < 6) return SEQ_PENDING;
            // ;
            if (seq[5] != ';') return SEQ_NONE;
            if (seq_sz < 7) return SEQ_PENDING;
            // Pv
            if (seq[4] != 3) { // ignore pv, expect S
                if (seq[6] == 'S') return Xterm_XTSMGRAPHICS;
                else return SEQ_NONE;
            } else {
                // A single integer or two integers
                if (seq[6] < '0' || seq[6] > '9') return SEQ_NONE;
                if (seq_sz < 8) return SEQ_PENDING;
                // important time point
                if (seq[7] == 'S') return Xterm_XTSMGRAPHICS;
                else if (seq[7] < '0' || seq[7] > '9') return SEQ_NONE;
                else {
                    // find a ;
                    size_t k = 8;
                    for (k; k < seq_sz; ++k) {
                        if (seq[k] == ';') break;
                        else if (seq[k] < '0' || seq[k] > '9')
                            return SEQ_NONE;
                    }
                    if (seq[k] != ';' || k == seq_sz) return SEQ_PENDING;
                    k++; // skip ;
                    for (k; k < seq_sz - 1; ++k) {
                        if (seq[k] < '0' || seq[k] > '9')
                            break;
                    }
                    if (k == seq_sz-1 && seq[k] == 'S') return Xterm_XTSMGRAPHICS;
                    else return SEQ_NONE;
                }
            }
        } else return SEQ_NONE;
    }
}
// CSI Ps T
// CSI Ps;Ps;Ps;Ps;Ps T
static inline int IsXterm_XTHIMOUSE(const std::string& seq)
{
    size_t seq_sz = seq.length();
	if (seq[0] != '[') return SEQ_NONE;
	if (seq_sz < 2) return SEQ_PENDING;
	{
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
                if (seq[b+1] == 'T') return Xterm_XTHIMOUSE;
            }
        }
    }
    return SEQ_NONE;
}
// CSI ? Ps i
static inline int IsXterm_Question_1Pm(const std::string& seq)
{
	size_t seq_sz = seq.length();
	if (seq[0] != '[') return SEQ_NONE;
	if (seq_sz < 2) return SEQ_PENDING;
	if (seq[1] != '?') return SEQ_NONE;
	if (seq_sz < 3) return SEQ_PENDING;{
        size_t b = 2;
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
            case 'h': return Xterm_DECSET;
            case 'i': return Xterm_DECMC;
            case 'l': return Xterm_DECRST;
            case 'r': return Xterm_XTRESTORE;
            case 's': return Xterm_XTSAVE;
            }
        }
    }
    return SEQ_NONE;
}
// CSI = Ps c
static inline int IsXterm_TertiaryDA(const std::string& seq)
{
	size_t seq_sz = seq.length();
	if (seq[0] != '[') return SEQ_NONE;
	if (seq_sz < 2) return SEQ_PENDING;
	if (seq[1] != '=') return SEQ_NONE;
	if (seq_sz < 3) return SEQ_PENDING;
	{
        size_t b = 2;
        while (b < seq_sz) {
            if (seq[b] >= '0' && seq[b] <= '9')
                b++;
            else
                break;
        }
        if (b == seq_sz)
            return SEQ_PENDING;
        else {
            if (seq[b] == 'c') return Xterm_TertiaryDA;
        }
    }
    return SEQ_NONE;
}
//
static inline int IsXterm_OSC(const std::string& seq)
{
	if (seq[0] == 0x5d) {
		if (seq.length() < 2) return SEQ_PENDING;
		else if (seq[seq.length()-1] == 0x07)
			return Xterm_OSC;
		else
			return SEQ_PENDING;
	}
	return SEQ_NONE;
}
static inline int IsXterm_1PmQuote(const std::string& seq)
{
	size_t seq_sz = seq.length();
	if (seq[0] != '[') return SEQ_NONE;
	if (seq_sz < 2) return SEQ_PENDING;
	{
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
            if (seq_sz - b == 1 && seq[b] == '\"') return SEQ_PENDING;
            else if (seq_sz - b == 2 && seq[b] == '\"') {
                switch (seq[b+1]) {
                case 'q': return Xterm_DECSCA;
                }
            }
        }
    }
    return SEQ_NONE;
}
static inline int IsXterm_2Ps(const std::string& seq)
{
	size_t seq_sz = seq.length();
	if (seq[0] != '[') return SEQ_NONE;
	if (seq_sz < 2) return SEQ_PENDING;
	{
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
                    case 's': return Xterm_DECSLRM;
                    }
                }
            } else
                return SEQ_NONE;
        }
    }
    return SEQ_NONE;
}
static inline int IsXterm_2PmQuote(const std::string& seq)
{
	size_t seq_sz = seq.length();
	if (seq[0] != '[') return SEQ_NONE;
	if (seq_sz < 2) return SEQ_PENDING;
	{
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
                    if (seq_sz - b == 1 && seq[b] == '\"') return SEQ_PENDING;
                    else if (seq_sz - b == 2 && seq[b] == '\"') {
                        switch (seq[b+1]) {
                        case 'p': return Xterm_DECSCL;
                        }
                    }
                }
            } else
                return SEQ_NONE;
        }
    }
    return SEQ_NONE;
}
// 1 or None Pm
static inline int IsXterm_1PmSharp(const std::string& seq)
{
    size_t seq_sz = seq.length();
	if (seq[0] != '[') return SEQ_NONE;
	if (seq_sz < 2) return SEQ_PENDING;
	{
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
            if (seq_sz - b == 1 && seq[b] == '#') return SEQ_PENDING;
            else if (seq_sz - b == 2 && seq[b] == '#') {
                switch (seq[b+1]) {
                case 'P': return Xterm_XTPUSHCOLORS;
                case 'Q': return Xterm_XTPOPCOLORS;
                case 'R': return Xterm_XTREPORTCOLORS;
                case 'p': return Xterm_XTPUSHSGR;
                }
            }
        }
    }
    return SEQ_NONE;
}
static inline int IsXterm_1PmDollar(const std::string& seq)
{
    size_t seq_sz = seq.length();
	if (seq[0] != '[') return SEQ_NONE;
	if (seq_sz < 2) return SEQ_PENDING;
	{
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
            if (seq_sz - b == 1 && seq[b] == '$') return SEQ_PENDING;
            else if (seq_sz - b == 2 && seq[b] == '$') {
                switch (seq[b+1]) {
                case 'p': return Xterm_DECRQM;
                }
            }
        }
    }
    return SEQ_NONE;
}

static inline int IsXterm_vPmDollar(const std::string& seq)
{
    size_t seq_sz = seq.length();
	if (seq[0] != '[') return SEQ_NONE;
	if (seq_sz < 2) return SEQ_PENDING;
	{
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
            if (seq_sz - b == 1 && seq[b] == '$') return SEQ_PENDING;
            else if (seq_sz - b == 2 && seq[b] == '$') {
                switch (seq[b+1]) {
                case 'r': return Xterm_DECCARA;
                }
            }
        }
    }
    return SEQ_NONE;
}

static inline int IsXterm_Question_1PmDollar(const std::string& seq)
{
    size_t seq_sz = seq.length();
	if (seq[0] != '[') return SEQ_NONE;
	if (seq_sz < 2) return SEQ_PENDING;
	if (seq[1] != '?') return SEQ_NONE;
	if (seq_sz < 3) return SEQ_PENDING;
	{
        size_t b = 2;
        while (b < seq_sz) {
            if (seq[b] >= '0' && seq[b] <= '9')
                b++;
            else
                break;
        }
        if (b == seq_sz)
            return SEQ_PENDING;
        else {
            if (seq_sz - b == 1 && seq[b] == '$') return SEQ_PENDING;
            else if (seq_sz - b == 2 && seq[b] == '$') {
                switch (seq[b+1]) {
                case 'p': return Xterm_DECRQPM;
                }
            }
        }
    }
    return SEQ_NONE;
}
static inline int IsXterm_1Ps(const std::string& seq)
{
	size_t seq_sz = seq.length();
	if (seq[0] != '[') return SEQ_NONE;
	if (seq_sz < 2) return SEQ_PENDING;
	{
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
            case 'q': return Xterm_DECLL;
            }
        }
    }
    return SEQ_NONE;
}
static inline int IsXterm_1PsB(const std::string& seq)
{
	size_t seq_sz = seq.length();
	if (seq[0] != '[') return SEQ_NONE;
	if (seq_sz < 2) return SEQ_PENDING;
	{
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
                case 'q': return Xterm_DECSCUSR;
                case 't': return Xterm_DECSWBV;
                }
            }
        }
    }
    return SEQ_NONE;
}
// CSI > Pp; Pv m
// CSI > Pp m
// CSI > Ps... [a,b,c.etc]
static inline int IsXterm_Gt_vPs(const std::string& seq)
{
	size_t seq_sz = seq.length();
	if (seq[0] != '[') return SEQ_NONE;
	if (seq_sz < 2) return SEQ_PENDING;
	if (seq[1] != '>') return SEQ_NONE;
	if (seq_sz < 3) return SEQ_PENDING;
	{
        size_t b = 2;
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
            case 'm': return Xterm_XTMODKEYS;
            case 'n': return Xterm_XTDISABLEMODKEYS;
            case 'p': return Xterm_XTSMPOINTER;
            case 'T': return Xterm_XTRMTITLE;
            case 't': return Xterm_XTSMTITLE;
            case 'c': return Xterm_SecondaryDA;
            case 'q': return Xterm_XTVERSION;
            }
        }
    }
    return SEQ_NONE;
}

static inline int IsXterm_vPs(const std::string& seq)
{
	size_t seq_sz = seq.length();
	if (seq[0] != '[') return SEQ_NONE;
	if (seq_sz < 2) return SEQ_PENDING;
	{
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
            case 't': return Xterm_XTWINOPS;
            }
        }
    }
    return SEQ_NONE;
}

static inline int IsXtermTrivialSeq(const std::string& seq)
{
    size_t vsz = sizeof(kXtermCtrlSeqs) / sizeof(kXtermCtrlSeqs[0]);
    size_t seq_sz = seq.length();
    for (size_t k = 0; k < vsz; ++k) {
        size_t pat_sz = strlen(kXtermCtrlSeqs[k]);
        if (seq_sz > pat_sz) continue; // try next
        if (strncmp(kXtermCtrlSeqs[k], seq.c_str(), seq_sz) == 0) {
            if (seq_sz < pat_sz) return SEQ_PENDING; // maybe, you should give more chars
            else if (seq_sz == pat_sz) return Xterm_Trivial; // It's a Xterm control seq
        }
    }
    return SEQ_NONE;
}
// match the control seq
// return 0 - It's not a Xterm control seq absolutely
//        1 - maybe, your should give more characters
//        2 - It's a Xterm control seq.
static inline int IsXtermControlSeq(const std::string& seq)
{
    std::function<int(const std::string&)> funcs[] = {
        IsXtermTrivialSeq,
        IsXterm_1Ps,
        IsXterm_1PsB,
        IsXterm_1PmSharp,
        IsXterm_1PmDollar,
        IsXterm_Question_1PmDollar,
        IsXterm_XTSMGRAPHICS,
        IsXterm_XTHIMOUSE,
        IsXterm_TertiaryDA,
        IsXterm_2PmQuote,
        IsXterm_Gt_vPs,
        IsXterm_OSC
    };
    int ret = SEQ_NONE;
    for (int i = 0; i < sizeof(funcs) / sizeof(funcs[0]); ++i) {
        ret = funcs[i](seq);
        if (ret == SEQ_NONE) continue; else break;
    }
    return ret;
}

#endif
