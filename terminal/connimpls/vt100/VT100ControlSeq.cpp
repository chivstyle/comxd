//
// (c) 2021 chiv
//
#include "VT100ControlSeq.h"

#define kESC  "\x1b"
#define kCSI  "\x1b\x5b"

void AddVT100ControlSeqs(ControlSeqFactory* factory)
{
    REGISTER_SEQ(factory, DECSM, "\x1b[?", Pn, 1, "h");
    REGISTER_SEQ(factory, DECRM, "\x1b[?", Pn, 1, "l");
    //
    REGISTER_SEQ(factory, DECKPAM,       kESC,     No, 0, "=");
    REGISTER_SEQ(factory, DECKPNM,       kESC,     No, 0, ">");
    //
    REGISTER_SEQ(factory, DECSTBM,       kCSI,     Pn, 2, "r");
    //
    REGISTER_SEQ(factory, G0_CS,         "\x1b(",  Gs, 0, "");
    REGISTER_SEQ(factory, G1_CS,         "\x1b)",  Gs, 0, "");
    //
    REGISTER_SEQ(factory, DECMC,  "\x1b[?", Pn, 0, "i");
    REGISTER_SEQ(factory, DECDSR, "\x1b[?", Ps, 0, "n");
    REGISTER_SEQ(factory, DECIND, kESC,     No, 0, "D");
    //
    REGISTER_SEQ(factory, DECREQTPARM, kCSI, Ps, 0, "x");
    //
    REGISTER_SEQ(factory, DECID,   kESC,     No, 0, "Z");
    REGISTER_SEQ(factory, DECSTBM, kCSI,     Pn, 2, "r");
    REGISTER_SEQ(factory, DECSC,   kESC,     No, 0, "7");
    REGISTER_SEQ(factory, DECRC,   kESC,     No, 0, "8");
    //
    REGISTER_SEQ(factory, DECDHT, "\x1b#", No, 0, "3");
    REGISTER_SEQ(factory, DECDHB, "\x1b#", No, 0, "4");
    REGISTER_SEQ(factory, DECSWL, "\x1b#", No, 0, "5");
    REGISTER_SEQ(factory, DECDWL, "\x1b#", No, 0, "6");
    //
    REGISTER_SEQ(factory, VT52_ENTER_ANSI_MODE,                      kESC, No, 0, "<");
    REGISTER_SEQ(factory, VT52_ENTER_ALTERNATE_KEYPAD_MODE,          kESC, No, 0, "=");
    REGISTER_SEQ(factory, VT52_ENTER_ALTERNATE_KEYPAD_MODE_NUMBERIC, kESC, No, 0, ">");
    REGISTER_SEQ(factory, VT52_GCS,  kESC, No, 0, "F"); // line drawing chars
    REGISTER_SEQ(factory, VT52_USUK, kESC, No, 0, "G");
    REGISTER_SEQ(factory, VT52_CUU,  kESC, No, 0, "A");
    REGISTER_SEQ(factory, VT52_CUD,  kESC, No, 0, "B");
    REGISTER_SEQ(factory, VT52_CUR,  kESC, No, 0, "C");
    REGISTER_SEQ(factory, VT52_CUL,  kESC, No, 0, "D");
    REGISTER_SEQ(factory, VT52_CUH,  kESC, No, 0, "H");
    REGISTER_SEQ(factory, VT52_CUP,  "\x1bY", Pn, 0, "\037");
    REGISTER_SEQ(factory, VT52_RI,   kESC, No, 0, "\111");
    //
    REGISTER_SEQ(factory, VT52_ERASE_TO_END_OF_LINE,          kESC, No, 0, "K");
    REGISTER_SEQ(factory, VT52_ERASE_TO_END_OF_SCREEN,        kESC, No, 0, "J");
    REGISTER_SEQ(factory, VT52_ENTER_AUTO_PRINT_MODE,         kESC, No, 0, "^");
    REGISTER_SEQ(factory, VT52_EXIT_AUTO_PRINT_MODE,          kESC, No, 0, "_");
    REGISTER_SEQ(factory, VT52_ENTER_PRINTER_CONTROLLER_MODE, kESC, No, 0, "W");
    REGISTER_SEQ(factory, VT52_EXIT_PRINTER_CONTROLLER_MODE,  kESC, No, 0, "X");
    REGISTER_SEQ(factory, VT52_PRTSC,                         kESC, No, 0, "]");
    REGISTER_SEQ(factory, VT52_PRTCL,                         kESC, No, 0, "V");
}
