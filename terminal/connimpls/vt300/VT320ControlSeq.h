//
// (c) 2021 chiv
//
#pragma once

#include "connimpls/ControlSeq.h"
#include <string>
//
enum VT320SeqType {
    DECSASD = VT300_SEQ_BEGIN,
    DECSSDT,
    DECRQTSR,
    DECRQPSR,
    DECRQM, // set mode dec private
    ANSIRQM, // set mode ansi
    DECRPM,
    DECRQUPSS,
    VT320_SEQ_MAX
};
static_assert(VT320_SEQ_MAX < VT300_SEQ_END, "You should define VT300_SEQ_MAX_COUNT big enough");

void AddVT320ControlSeqs(ControlSeqFactory* factory);