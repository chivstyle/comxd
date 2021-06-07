//
// (c) 2021 chiv
//
#pragma once

#include "connimpls/ControlSeq.h"
#include <string>
//
enum VT420SeqType {
    // DECDMAC, A DCS STRING
    DECINVM = VT300_SEQ_BEGIN,
    ANSI_CONFORMANCE_LEVEL_1,
    ANSI_CONFORMANCE_LEVEL_2,
    ANSI_CONFORMANCE_LEVEL_3,
    DECSLPP,
    DECSLRM,
    // DECVSSM, DECSM/DECRM will process it
    // Rectangular Area Operations
    DECCRA,
    DECERA,
    DECFRA,
    DECSERA,
    DECSACE,
    DECCARA,
    DECRARA,
    //
    DECBI,
    DECFI,
    //
    DECSNLS,
    //
    TertiaryDA,
    //
    DECRQCRA, // response is DECCKSR
    DECRQDE,
    //
    ENABLE_SESSION,
    //
    VT420_SEQ_MAX
};
static_assert(VT420_SEQ_MAX < VT400_SEQ_END, "You should define VT400_SEQ_MAX_COUNT big enough");

void AddVT420ControlSeqs(ControlSeqFactory* factory);