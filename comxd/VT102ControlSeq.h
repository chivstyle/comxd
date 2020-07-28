//
// (c) 2020 chiv
//
#ifndef _comxd_VT102ControlSeq_h_
#define _comxd_VT102ControlSeq_h_

#include <string>
/// VT102 Control Codes
/// please read [VT102 Manual](https://vt100.net/docs/vt102-ug/appendixc.html)
static const char* kVT102CtrlSeqs[] = {
    "[2h",
    "[4h",
    "[12h",
    "[20h",
    "[?1h",
    "[?3h",
    "[?4h",
    "[?5h",
    "[?6h",
    "[?7h",
    "[?8h",
    "[?18h",
    "[?19h",
    "[2l",
    "[4l",
    "[12l",
    "[20l",
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
    "[=",
    "[>",
    "?p",
    "?q",
    "?r",
    "?t",
    "?u",
    "?v",
    "?w",
    "?x",
    "?y",
    "?m",
    "?l*",
    "?n",
    "?M",
    "P",
    "Q",
    "R",
    "S",
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
    "O",
    "[m",
    "[0m",
    "[1m",
    "[4m",
    "[5m",
    "[7m",
#if 0
    // Cursor key codes
    "[A",
    "[0A",
    "[B",
    "[0B",
    "[C",
    "[0C",
    "[D",
    "[0D",
#endif
    // Cursor movement commands
    "[Pt:Pbr",
    "[PnA",
    "[PnB",
    "[PnC",
    "[PnD",
    "[Pl;PcH",
    "[H",
    "[Pl;Pcf]",
    "[f",
    "D",
    "M",
    "E",
    "7",
    "8",
    // Tab stops
    "H",
    "[g",
    "[0g",
    "[3g",
    "#3",
    "#4",
    "#5",
    "#6",
    // Erasing
    "[K",
    "[0K",
    "[1K",
    "[2K",
    "[J",
    "[0J",
    "[1J",
    "[2J",
    // Editing functions
    "[PnP",
    "[PnL",
    "[PnM",
    // Print commands
    "[?5i",
    "[?4i",
    "[5i",
    "[4i",
    "[i",
    "[0i",
    "[?1i",
    // Reports
    "[5n",
    "[0n",
    "[3n",
    "[?15n",
    "[?10n",
    "[?11n",
    "[?13n",
    "[6n",
    "[Pl;PcR",
    "[c",
    "[0c",
    "Z",
    "[?6c",
    // Reset
    "c",
    // Tests and adjustments
    "#8",
    "[2;1y",
    "[2;2y",
    "[2;4y",
    "[2;9y",
    "[2;10y",
    "[2;12y",
    "[2;16y",
    "[2;24y",
    // Keyboard LED(s)
    "[q",
    "[0q",
    "[1q",
    // VT52 compatible mode
    "<",
    // Keypad character selection
    "=",
    ">",
    // Character sets
    "F*",
    "G",
    // Cursor position
    "A",
    "B",
    "C",
    "D",
    "H",
    "YPlPc\037",
    "I\x49", /// o'111
    // Erasing
    "K",
    "J",
    // Print commands
    "^",
    "_",
    "W",
    "X",
    "]",
    "V",
    // Reports
    "Z",
    "/Z"
};
static inline int IsVT102CursorKeyCodes(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz > 2) {
        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                bool nb = true;
                size_t b = 2;
                while (b < seq_sz-1) {
                    if (!(seq[b] >= '0' && seq[b] <= '9')) {
                        nb = false;
                        break;
                    }
                    b++;
                }
                if (nb) { // 1 ~ n-1 are all digits, if the last char is 'A' or 'B' or 'C' or 'D'
                          // this seq is a cursor key codes
                    if (seq[b] == 'A' || seq[b] == 'B' || seq[b] == 'C' || seq[b] == 'D') {
                        return 2; // dynamic cursor position key codes
                    } else if (seq[b] >= '0' && seq[b] <= '9') {
                        return 1;
                    }
                } else {
                    return 0;
                }
            }
        }
    } else if (seq_sz == 2 && seq[0] == '[' && seq[1] >= '0' && seq[1] <= '9') {
        return 1;
    } else if (seq_sz == 1 && seq[0] == '[') return 1;
    //
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
