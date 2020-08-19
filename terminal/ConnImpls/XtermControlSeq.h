//
// (c) 2020 chiv
//
#ifndef _comxd_XtermControlSeq_h_
#define _comxd_XtermControlSeq_h_

#include "ControlSeq.h"

// [http://xfree86.org/4.7.0/ctlseqs.html]
static const char* kXtermCtrlSeqs[] = {
    "[?1049h",
    "[?1049l"
};

enum XtermSeqType {
    Xterm_Trivial = XTERM_SEQ_BEGIN,
    Xterm_SeqType_Endup
};

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
        IsXtermTrivialSeq
    };
    int ret = SEQ_NONE;
    for (int i = 0; i < sizeof(funcs) / sizeof(funcs[0]); ++i) {
        ret = funcs[i](seq);
        if (ret == SEQ_NONE) continue; else break;
    }
    return ret;
}

#endif
