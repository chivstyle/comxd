//
// (c) 2021 chiv
//
#pragma once

#include "connimpls/ControlSeq.h"
#include <string>
//
enum VT200SeqType {
    DECSCL = VT200_SEQ_BEGIN,
    G2_CS,
    G3_CS,
    S7C1T,
    S8C1T,
    DECKPAM,
    DECKPNM,
    DECSCA,
    DECSEL,
    DECSED,
    PRINTING,
    SecondaryDA,
    DECSTR,
    
    VT200_SEQ_MAX
};
static_assert(VT200_SEQ_MAX < VT200_SEQ_END, "You should define VT200_SEQ_MAX_COUNT big enough");

void AddVT200ControlSeqs(ControlSeqFactory* factory);