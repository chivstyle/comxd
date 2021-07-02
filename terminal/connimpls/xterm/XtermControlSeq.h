//
// (c) 2021 chiv
//
#pragma once

#include "connimpls/ControlSeq.h"
#include <string>
//
enum XtermSeqType {
    XTPUSHCOLORS = XTERM_SEQ_BEGIN,
    XTPOPCOLORS,
    XTREPORTCOLORS,
    XTSMGRAPHICS,
    XTHIMOUSE,
    XTRMTITLE,
    XTMODKEYS,
    XTSMPOINTER,
    XTPUSHSGR,
    XTVERSION,
    XTPOPSGR,
    XTRESTORE,
    XTSAVE,
    XTWINOPS,
    XTSMTITLE,
    XTCHECKSUM,
    XTREPORTSGR,
    // xterm OSC, terminate with bell(0x07)
    XTOSC,
    
    Xterm_SEQ_MAX
};
static_assert(Xterm_SEQ_MAX < XTERM_SEQ_END, "You should define XTERM_SEQ_MAX_COUNT big enough");

void AddXtermControlSeqs(ControlSeqFactory* factory);