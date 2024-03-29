//
// (c) 2021 chiv
//
#pragma once

#include "connimpls/ControlSeq.h"
#include <string>

namespace xvt {
enum VT100SeqType {
    // DEC Private Modes, others were included in ECMA-48
    DECSM = VT100_SEQ_BEGIN,
    DECRM,
    // DEC Private mode
    DECKPAM,
    DECKPNM,
    //
    G0_CS,
    G1_CS,
    // I hate naming...
    DECMC,
    DECDSR,
    DECIND,
    //
    DECREQTPARM,
    DECID,
    DECSTBM,
    DECSC,
    DECRC,
    DECDHT,
    DECDHB,
    DECSWL,
    DECDWL,
    DECALN,
    DECTST,
    DECLL,
    // VT52 Compatible
    VT52_ENTER_ANSI_MODE,
    VT52_ENTER_ALTERNATE_KEYPAD_MODE,
    VT52_ENTER_ALTERNATE_KEYPAD_MODE_NUMBERIC,
    VT52_GCS,
    VT52_USUK,
    VT52_CUU,
    VT52_CUD,
    VT52_CUR,
    VT52_CUL,
    VT52_CUH,
    VT52_CUP,
    VT52_RI,
    VT52_ERASE_TO_END_OF_LINE,
    VT52_ERASE_TO_END_OF_SCREEN,
    VT52_ENTER_AUTO_PRINT_MODE,
    VT52_EXIT_AUTO_PRINT_MODE,
    VT52_ENTER_PRINTER_CONTROLLER_MODE,
    VT52_EXIT_PRINTER_CONTROLLER_MODE,
    VT52_PRTSC, // print screen
    VT52_PRTCL, // print cursor line
    //
    VT100_SEQ_MAX
};
static_assert(VT100_SEQ_MAX < VT100_SEQ_END, "You should define VT100_SEQ_MAX_COUNT big enough");

void AddVT100ControlSeqs(ControlSeqFactory* factory);
}
