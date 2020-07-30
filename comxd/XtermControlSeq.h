//
// (c) 2020 chiv
//
#ifndef _comxd_XtermControlSeq_h_
#define _comxd_XtermControlSeq_h_


#include <string>
// ECMA-48, .etc
static const char* kXtermCtrlSeqs[] = {
    "[30m", // black
    "[31m", // red
    "[32m", // green
    "[33m", // yellow
    "[34m", // blue
    "[35m", // magenta
    "[36m", // cyan
    "[37m", // white
    "[38m", // reserved for future standardization
    "[39m", // default color
    "[40m", // black background
    "[41m", // red background
    "[42m", // green background
    "[43m", // yellow background
    "[44m", // blue background
    "[45m", // magenta background
    "[46m", // cyan background
    "[47m", // black background
    "[48m", // reserved for future standardization
    "[49m", // default background
    "[1;0m",
    "[1;30m", // black
    "[1;31m", // red
    "[1;32m", // green
    "[1;33m", // yellow
    "[1;34m", // blue
    "[1;35m", // magenta
    "[1;36m", // cyan
    "[1;37m", // white
    "[1;38m", // reserved for future standardization
    "[1;39m", // default color
    "[1;40m", // black background
    "[1;41m", // red background
    "[1;42m", // green background
    "[1;43m", // yellow background
    "[1;44m", // blue background
    "[1;45m", // magenta background
    "[1;46m", // cyan background
    "[1;47m", // black background
    "[1;48m", // reserved for future standardization
    "[1;49m", // default background
    "[0;0m",
    "[0;30m", // black
    "[0;31m", // red
    "[0;32m", // green
    "[0;33m", // yellow
    "[0;34m", // blue
    "[0;35m", // magenta
    "[0;36m", // cyan
    "[0;37m", // white
    "[0;38m", // reserved for future standardization
    "[0;39m", // default color
    "[0;40m", // black background
    "[0;41m", // red background
    "[0;42m", // green background
    "[0;43m", // yellow background
    "[0;44m", // blue background
    "[0;45m", // magenta background
    "[0;46m", // cyan background
    "[0;47m", // black background
    "[0;48m", // reserved for future standardization
    "[0;49m", // default background
};
// match the control seq
// return 0 - It's not a Xterm control seq absolutely
//        1 - maybe, your should give more characters
//        2 - It's a Xterm control seq.
static inline int IsXtermControlSeq(const std::string& seq)
{
    size_t vsz = sizeof(kXtermCtrlSeqs) / sizeof(kXtermCtrlSeqs[0]);
    size_t seq_sz = seq.length();
    for (size_t k = 0; k < vsz; ++k) {
        size_t pat_sz = strlen(kXtermCtrlSeqs[k]);
        if (seq_sz > pat_sz) continue; // try next
        if (strncmp(kXtermCtrlSeqs[k], seq.c_str(), seq_sz) == 0) {
            if (seq_sz < pat_sz) return 1; // maybe, you should give more chars
            else if (seq_sz == pat_sz) return 2; // It's a Xterm control seq
        }
    }
    return 0;
}

#endif
