//
// (c) 2021 chiv
//
#pragma once

#include "connimpls/ControlSeq.h"
#include <string>
//
enum VT520SeqType {
    // Ignore these please
    DDD1,
    DDD2,
    DDD3,
    //
    DECAC,
    DECARR, // select auto repeat rate
    DECATC, // alternate text color
    DECCKSR, // memory checksum report
    DECCRTST, // CRT saver timing
    DECDHLT, // double width, double height, TOP
    DECDHLB, // double width, double height, BOTTOM
    DECDLDA, // down line load allocation
    DECKBD, // keyboard language selection
    DECLTOD, // load time of day
    DECMSR, // macro space report
    DECPCTERM, // enter/exit PCTerm or Scancode mode
    DECPKA, // program key action
    DECPS, // play sound
    DECRQKD, // request key definition
    DECRQKT, // request key type
    DECRQM_ANSI, // request ansi mode
    DECRQM_DECP, // request dec private mode
    
    VT520_SEQ_MAX
};
static_assert(VT520_SEQ_MAX < VT500_SEQ_END, "You should define VT500_SEQ_MAX_COUNT big enough");

void AddVT520ControlSeqs(ControlSeqFactory* factory);