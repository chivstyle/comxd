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
    DECRQPKFM, // request program key free memory
    DECCTR, // color table request
    DECSCP, // select communication port
    DECSCS, // select communication speed
    DECSCUSR, // set cursor style
    DECSDDT, // select disconnect delay time
    DECSDPT, // Select Digital Printed Data Type
    DECSEST, // Energy saver timing
    DECSFC, // Select Flow Control
    DECSKCV, // Set Key Click Volume
    DECSLCK, // Set Lock Key Style
    DECSMBV, // Set Margin Bell Volume
    DECSPMA, // Session Page Memory Allocation
    DECSPP, // Set Port Parameters
    DECSPPCS, // Select ProPrinter Character Set
    DECSPRTT, // Select Printer Type
    DECSR, // Secure Reset
    DECSRC, // Secure Reset Confirmation
    DECSSCLS, // Set Scroll Speed
    DECSSL, // Select Set-Up Language
    DECST8C, // Set Tab at Every 8 Columns
    DECSTGLT, // Select Color Look-Up Table
    DECSTRL, // Set Transmit Rate Limit
    DECSWBV, // Set Warning Bell Volume
    DECSZS, // Select Zero Symbol
    DECTID, // Select Terminal ID
    DECTME, // Terminal Mode Emulation
    DECUS, // Update Session
    
    VT520_SEQ_MAX
};
static_assert(VT520_SEQ_MAX < VT500_SEQ_END, "You should define VT500_SEQ_MAX_COUNT big enough");

void AddVT520ControlSeqs(ControlSeqFactory* factory);