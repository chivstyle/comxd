//
// (c) 2021 chiv
//
#pragma once

#include "connimpls/ControlSeq.h"
#include <string>
//
enum VT220SeqType {
    DECSCL = VT200_SEQ_BEGIN,
    G2_CS,
    G3_CS,
    S7C1T,
    S8C1T,
    DECSCA,
    DECSEL,
    DECSED,
    PRINTING,
    SecondaryDA,
    DECSTR,
    VT220_SEQ_MAX
};
static_assert(VT220_SEQ_MAX < VT200_SEQ_END, "You should define VT200_SEQ_MAX_COUNT big enough");

void AddVT220ControlSeqs(ControlSeqFactory* factory);