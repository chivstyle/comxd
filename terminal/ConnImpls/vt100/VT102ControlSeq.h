//
// (c) 2021 chiv
//
#pragma once

#include "connimpls/ControlSeq.h"
#include <string>
//
enum VT102SeqType {
	// ANSI-Compatible Private Modes, others were included in ECMA-48
    VT102_MODE_SET = VT102_SEQ_BEGIN,
    VT102_MODE_RESET,
    //
    DECKPAM,
    DECKPNM,
    //
    VT102_G0_UK, VT102_G1_UK,
    VT102_G0_US, VT102_G1_US,
    VT102_G0_LINE_DRAWING, VT102_G1_LINE_DRAWING,
    VT102_G0_ROM, VT102_G1_ROM,
    VT102_G0_ROM_SPECIAL, VT102_G1_ROM_SPECIAL,
    //
    VT102_MC,
    VT102_DSR,
    VT102_IND,
    //
    DECID,
    //
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
    VT102_SEQ_MAX
};
static_assert(VT102_SEQ_MAX < VT102_SEQ_END, "You should define VT102_SEQ_MAX_COUNT big enough");

void AddVT102ControlSeqs(ControlSeqFactory* factory);