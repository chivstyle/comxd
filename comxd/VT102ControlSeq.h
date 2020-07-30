//
// (c) 2020 chiv
//
#ifndef _comxd_VT102ControlSeq_h_
#define _comxd_VT102ControlSeq_h_

#include "ControlSeq.h"
#include <string>
/// WE DO NOT SUPPORT VT52 compatible mode
/// VT102 Control Codes
/// please read [VT102 Manual](https://vt100.net/docs/vt102-ug/appendixc.html)
static const char* kVT102CtrlSeqs[] = {
    // Print commands
    // 1. ANSI Compatible Seq
    // 1.1 set mode
    "[2h",
    "[4h",
    "[12h",
    "[20h",
    "[?1h",
    "[?3h", // 180 columns
    "[?4h",
    "[?5h",
    "[?6h",
    "[?7h",
    "[?8h",
    "[?18h",
    "[?19h",
    // 1.2 reset mode
    "[2l", // keyboard action
    "[4l", // insertion-replacement
    "[12l", // send-receive
    "[20l", // linefeed/newline
    "[?1l",
    "[?2l",
    "[?3l", // 80 columns
    "[?4l",
    "[?5l",
    "[?6l",
    "[?7l",
    "[?8l",
    "[?18l",
    "[?19l",
    // 1.3 Cursor Key Codes Generated
    "[A","0A",
    "[B","0B",
    "[C","0C",
    "[D","0D",
    // 1.4 Keypad character selection
    "=", // alternate
    ">", // numeric
    // 1.5 Keypad codes generated
    "?p", "?q", "?r", "?s", "?t", "?u", "?v", "?w", "?x", "?y", "?m",
    "?l\154", "?n", "?M", "P", "Q", "R", "S",
    "0p", "0q", "0r", "0s", "0t", "0u", "0v", "0w", "0x", "0y", "0m",
    "0l\154", "0n", "0M", "0P", "0Q", "0R", "0S",
    // 1.6 select character sets
    "(A",")A",
    "(B",")B",
    "(0",")0",
    "(1",")1",
    "(2",")2",
    "N",
    "O",
    // 1.7 character attributes
    "[m",
    "[0m",
    "[1m",
    "[4m",
    "[5m",
    "[7m",
    //-----------------------------------------------------
    // 1.9 scrolling region, see IsVT102ScrollingRegion
    //-----------------------------------------------------
    // 1.10 Cursor movement commands
    "[H", // home
    "[f", // horizontal and vertical position HOME
    "D", // index
    "M", // reverse index
    "E", // next line
    "7", // save cursor (and attributes)
    "8", // restore cursor (and attributes)
    // others, see IsVT102CursorKeyCodes
    //-----------------------------------------------------
    // 1.11 Tab stops
    "H", // horizontal tab set (at current column)
    "[g", // tabulation clear (at current column)
    "[0g", // see above
    "[3g", // tabulation clear (all tabs)
    // 1.12 Line attributes
    "#3", // double-height top half
    "#4", // double-height bottom half
    "#5", // single-width single-height
    "#6", // double-width single-height
    // 1.13 Erasing
    "[K",
    "[0K",
    "[1K",
    "[2K",
    "[J",
    "[0J",
    "[1J",
    "[2J",
    //-----------------------------------------------------
    // 1.14 Editing functions, see IsVT102EditingFunctions
    // [Pn P], [Pn L], [Pn M]
    //-----------------------------------------------------
    // 1.15 Print commands
    "[?5i",
    "[?4i",
    "[5i",
    "[4i",
    "[i", "[0i",
    "[?1i",
    // 1.16 Reports
    "[5n", // device status report
    "[0n",
    "[3n",
    "[?15n",
    "[?10n",
    "[?11n",
    "[?13n",
    "[6n",
    "[c", // what are you?
    "[0c", // same as [c
    "Z", // Identify terminal (what are you), not recommended
    "?6c", // device attributes response:VT102
    // 1.17 Reset
    "c", // reset to initial state
    // 1.18 Test and adjustments
    "#8", // fill screen with "Es"
    "[2;1y",
    "[2;2y",
    "[2;4y",
    "[2;9y",
    "[2;10y",
    "[2;12y",
    "[2;16y",
    "[2;24y",
    // 1.19 keyboard led
    "[q",
    "[0q",
    "[1q",
    // 2. VT52 Compatible Mode
    "<", // enter ANSI mode
    // 2.1 keypad character selection
    "=",
    ">",
    // 2.2 character sets
    "F",
    "G",
    // 2.3 cursor position
    "A",
    "B",
    "C",
    "D",
    "H",
    "I\111",
    // others, see IsVT52CursorKeyCodes
    //-----------------------------------------------------
    // 2.4 Erasing
    "K",
    "J",
    // 2.5 Print Commands
    "^",
    "_",
    "W",
    "X",
    "]",
    "V",
    "Z",
    "/Z"
};
//
static inline int IsVT102EditingFunctions(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[') return 1; // pending
    else if (seq_sz == 2 && seq[0] == '[' && seq[1] >= '0' && seq[1] <= '9') return 1; // pending
    else {
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
                if (seq[b] == 'P' || seq[b] == 'L' || seq[b] == 'M') return VT102_EditingFunctions;
                else return 0;
            }
        }
    }
    return 0;
}
//
static inline int IsVT52CursorKeyCodes(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == 'Y') return 1; // pending
    else if (seq_sz == 2 && seq[0] == 'Y' && seq[1] >= '0' && seq[1] <= '9') return 1; // pending
    else {
        if (seq[0] == 'Y' && seq[1] >= '0' && seq[1] <= '9') {
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
                if (seq[b] != ' ') return 0; else {
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
                // r - request
                // R - response
                else if (b < seq_sz && (seq[b] == '\037')) return VT52_Cursor;
                else return 0;
            }
        }
    }
    return 0;
}
//
static inline int IsVT102ScrollingRegion(const std::string& seq)
{
    size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[') return 1; // pending
    else if (seq_sz == 2 && seq[0] == '[' && seq[1] >= '0' && seq[1] <= '9') return 1; // pending
    else {
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
                // r - request
                // R - response
                else if (b < seq_sz && (seq[b] == 'r' || seq[b] == 'R')) return VT102_ScrollingRegion;
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
                if (seq[b] == 'A' || seq[b] == 'B' || seq[b] == 'C' || seq[b] == 'D') return VT102_Cursor;
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
                else if (b < seq_sz && (seq[b] == 'H' || seq[b] == 'f')) return VT102_Cursor;
                else return 0;
            }
        }
    }
    return 0;
}
// match the control seq
// return 0 - It's not a VT102 control seq absolutely
//        1 - maybe, your should give more characters
//        2 - It's a trivial VT102 control seq
static inline int IsVT102ControlSeq(const std::string& seq)
{
    int ret = IsVT102CursorKeyCodes(seq);
    if (ret == 0) {
        ret = IsVT102EditingFunctions(seq);
        if (ret == 0) {
            ret = IsVT52CursorKeyCodes(seq);
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
        }
    }
    return ret;
}

#endif
