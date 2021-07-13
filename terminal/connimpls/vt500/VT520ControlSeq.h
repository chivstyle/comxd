//
// (c) 2021 chiv
//
#pragma once

#include "connimpls/ControlSeq.h"
#include <string>
//
enum VT520SeqType {
    DECSWBV = VT500_SEQ_BEGIN,
    DECSZS,
    DECAC,
    VT520_SEQ_MAX
};
static_assert(VT520_SEQ_MAX < VT500_SEQ_END, "You should define VT500_SEQ_MAX_COUNT big enough");

void AddVT520ControlSeqs(ControlSeqFactory* factory);