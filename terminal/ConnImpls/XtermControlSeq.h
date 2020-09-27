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
    //-----------------------------------------------------------------
    // Application Program-Command functions
    // Xterm implements no APC functions.
    //-----------------------------------------------------------------
    // Device-Control functions
    // TODO:
    //-----------------------------------------------------------------
    // Functions using CSI, most of them are compatible with ecma-48
};

enum XtermSeqType {
    Xterm_Trivial = XTERM_SEQ_BEGIN,
    Xterm_XTPUSHCOLORS,
    Xterm_XTPOPCOLORS,
    Xterm_XTREPORTCOLORS,
    Xterm_SeqType_Endup
};

static_assert(Xterm_SeqType_Endup < XTERM_SEQ_END, "Xterm_SeqType_Endup should be less than XTERM_SEQ_END");
// 1 or None Pm
static inline int IsXterm_1PmSharp(const std::string& seq)
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
                if (seq_sz - b == 1 && seq[b] == '#') return SEQ_PENDING;
                else if (seq_sz - b == 2 && seq[b] == '#') {
                    switch (seq[b+1]) {
                    case 'P': return Xterm_XTPUSHCOLORS;
                    case 'Q': return Xterm_XTPOPCOLORS;
                    case 'R': return Xterm_XTREPORTCOLORS;
                    }
                }
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
        IsXterm_1PmSharp
    };
    int ret = SEQ_NONE;
    for (int i = 0; i < sizeof(funcs) / sizeof(funcs[0]); ++i) {
        ret = funcs[i](seq);
        if (ret == SEQ_NONE) continue; else break;
    }
    return ret;
}

#endif
