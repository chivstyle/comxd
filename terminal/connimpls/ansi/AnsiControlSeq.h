//
// (c) 2021 chiv
//
#pragma once

#include "connimpls/ControlSeq.h"
#include <string>
//
enum AnsiSeqType {
    ANSI_SCP = ANSI_SEQ_BEGIN,
    ANSI_RCP,
    //
    ANSI_SEQ_MAX
};
static_assert(ANSI_SEQ_MAX < ANSI_SEQ_END, "You should define ANSI_SEQ_MAX_COUNT big enough");

void AddAnsiControlSeqs(ControlSeqFactory* factory);