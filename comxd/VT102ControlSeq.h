//
// (c) 2020 chiv
//
#ifndef _comxd_VT102ControlSeq_h_
#define _comxd_VT102ControlSeq_h_

#include <string>
/// WE DO NOT SUPPORT VT52 compatible mode
/// VT102 Control Codes
/// please read [VT102 Manual](https://vt100.net/docs/vt102-ug/appendixc.html)
static const char* kVT102CtrlSeqs[] = {
    // attributes
    "[m",
    "[0m",
    "[1m",
    "[4m",
    "[5m",
    "[7m",
    // Cursor movement commands, Pt, Pb, Pn, .etc are parameters
    "[H", // home
    "[f", // horizontal and vertical position HOME
    "[A",
    "[B",
    "[C",
    "[D",
    "D", // index
    "M", // reverse index
    "E", // next line
    "7", // save cursor (and attributes)
    "8", // restore cursor (and attributes)
    // Tab stops
    "H", // horizontal tab set (at current column)
    "[g", // tabulation clear (at current column)
    "[0g", // see above
    "[3g", // tabulation clear (all tabs)
    // Line attributes
    "#3", // double-height top half
    "#4", // double-height bottom half
    "#5", // single-width single-height
    "#6", // double-width single-height
    // Erasing
    "[K",
    "[0K",
    "[1K",
    "[2K",
    "[J",
    "[0J",
    "[1J",
    "[2J",
    // reset mode
    "[2l", // keyboard action
    "[4l", // insertion-replacement
    "[12l", // send-receive
    "[20l", // linefeed/newline
    "[?1l",
    "[?2l",
    "[?3l",
    "[?4l",
    "[?5l",
    "[?6l",
    "[?7l",
    "[?8l",
    "[?18l",
    "[?19l",
    // select character sets
    "(A",
    ")A",
    "(B",
    ")B",
    "(0",
    ")0",
    "(1",
    ")1",
    "(2",
    ")2",
    "N",
    "O"
};
//
static inline int IsVT102ScrollingRegion(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[') return 1; // pending
    else if (seq_sz == 2 && seq[0] == '[' && seq[1] >= '0' && seq[1] <= '9') return 1; // pending
    else {
        // [Pt; Pb r]
        if (seq[0] == '[' && seq[1] >= '0' && seq[1] <= '9') {
            bool nb = true;
            size_t b = 2;
            while (b < seq_sz) {
                if (!(seq[b] >= '0' && seq[b] <= '9')) {
                    nb = false;
                    break;
                }
                b++;
            }
            // contiguous numbers
            if (nb) return 1; // pending
            else if (b < seq_sz) {
                nb = true;
                if (seq[b] != ';') return 0; else {
                    b++; // skip ;
                    while (b < seq_sz) {
                        if (!(seq[b] >= '0' && seq[b] <= '9')) {
                            nb = false;
                            break;
                        }
                        b++;
                    }
                }
                if (nb) return 1;
                else if (b < seq_sz && seq[b] == 'r') return 2;
                else return 0;
            }
        }
    }
    return 0;
}

static inline int IsVT102CursorKeyCodes(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[') return 1; // pending
    else if (seq_sz == 2 && seq[0] == '[' && seq[1] >= '0' && seq[1] <= '9') return 1; // pending
    else {
        // [Pt; Pb r]
        if (seq[0] == '[' && seq[1] >= '0' && seq[1] <= '9') {
            bool nb = true;
            size_t b = 2;
            while (b < seq_sz) {
                if (!(seq[b] >= '0' && seq[b] <= '9')) {
                    nb = false;
                    break;
                }
                b++;
            }
            // contiguous numbers
            if (nb) return 1; // pending
            else if (b < seq_sz) {
                if (seq[b] == 'A' || seq[b] == 'B' || seq[b] == 'C' || seq[b] == 'D') return 2;
                if (seq[b] != ';') return 0; else {
                    b++; // skip ;
                    nb = true;
                    while (b < seq_sz) {
                        if (!(seq[b] >= '0' && seq[b] <= '9')) {
                            nb = false;
                            break;
                        }
                        b++;
                    }
                }
                if (nb) return 1;
                else if (b < seq_sz && seq[b] == 'H' || seq[b] == 'f') return 2;
                else return 0;
            }
        }
    }
    return 0;
}
// match the control seq
// return 0 - It's not a VT102 control seq absolutely
//        1 - maybe, your should give more characters
//        2 - It's a VT102 control seq.
static inline int IsVT102ControlSeq(const std::string& seq)
{
    int ret = IsVT102CursorKeyCodes(seq);
    if (ret == 0) {
        size_t vsz = sizeof(kVT102CtrlSeqs) / sizeof(kVT102CtrlSeqs[0]);
        size_t seq_sz = seq.length();
        for (size_t k = 0; k < vsz; ++k) {
            size_t pat_sz = strlen(kVT102CtrlSeqs[k]);
            if (seq_sz > pat_sz) continue; // try next
            if (strncmp(kVT102CtrlSeqs[k], seq.c_str(), seq_sz) == 0) {
                if (seq_sz < pat_sz) return 1; // maybe, you should give more chars
                else if (seq_sz == pat_sz) return 2; // It's a VT102 control seq
            }
        }
    }
    return ret;
}

#endif
