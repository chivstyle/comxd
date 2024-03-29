//
// (c) 2021 chiv
//
#pragma once

#include "connimpls/ControlSeq.h"
#include <string>

namespace xvt {
//
enum XtermSeqType {
    XTPUSHCOLORS = XTERM_SEQ_BEGIN,
    XTPOPCOLORS,
    XTSMGRAPHICS,
    XTHIMOUSE,
    XTRMTITLE,
    XTMODKEYS,
    XTSMPOINTER,
    XTPUSHSGR,
    XTPOPSGR,
    XTVERSION,
    XTRESTORE,
    XTSAVE,
    XTWINOPS,
    XTSMTITLE,
    XTCHECKSUM,
    XTREPORTCOLORS,
    XTREPORTSGR,
    XTDISABLEMODOPTS,
    // xterm OSC, terminate with bell(0x07)
    XTOSC,
    // ECMA-43
    ANSI_CONFORMANCE_LEVEL1,
    ANSI_CONFORMANCE_LEVEL2,
    ANSI_CONFORMANCE_LEVEL3,
    // ignore all those seqs
    XTIGNORE,

    Xterm_SEQ_MAX
};
static_assert(Xterm_SEQ_MAX < XTERM_SEQ_END, "You should define XTERM_SEQ_MAX_COUNT big enough");

void AddXtermControlSeqs(ControlSeqFactory* factory);

}
