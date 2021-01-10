//
// (c) 2021 chiv
//
#pragma once

#include "connimpls/ControlSeq.h"
#include <string>
//
enum Ecma48SeqType {
    // VT102
    
    // Alpha private
    //
    VT102_SEQ_COUNT
};
static_assert(VT102_SEQ_COUNT < VT102_SEQ_MAX_COUNT, "You should define VT102_SEQ_MAX_COUNT big enough");

void AddVT102ControlSeqs(ControlSeqFactory* factory);