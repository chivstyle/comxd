/*!
// (c) 2021 chiv
*/
#include "XtermControlSeq.h"

// HELP
// CSI           "\E["
// OSC           "\E]
// DEC private   "\E[?"
//
void AddXtermControlSeqs(ControlSeqFactory* factory)
{
    REGISTER_SEQ(factory, XTWINOPS,       "\E[",  Ps, 0, "t");
    REGISTER_SEQ(factory, XTOSC,          "\E]",  Gs, 0, "\x07");
    // For convenience, we do not use strict parameter conditions, so
    // "\E\>1T", "\E\>1;2;3T", .etc were parsed as A single seq, the ECMA-048
    // defines it to SD, if there are three Ps in the parameter, you should
    // treat the seq as XTRMTITLE. please override ProcessSD to implement this feature.
    REGISTER_SEQ(factory, XTRMTITLE,      "\E[>", Ps, 0, "T");
    REGISTER_SEQ(factory, XTSMTITLE,      "\E[>", Ps, 0, "t");
    REGISTER_SEQ(factory, XTMODKEYS,      "\E[>", Ps, 0, "m");
    REGISTER_SEQ(factory, XTSMPOINTER,    "\E[>", Ps, 0, "p");
    REGISTER_SEQ(factory, XTVERSION,      "\E[>", Ps, 0, "q");
    REGISTER_SEQ(factory, XTPUSHCOLORS,   "\E[",  Ps, 0, "#P");
    REGISTER_SEQ(factory, XTPOPCOLORS,    "\E[",  Ps, 0, "#Q");
    // The 2 seqs below are responses
    REGISTER_SEQ(factory, XTREPORTCOLORS, "\E[",  No, 0, "#R");
    REGISTER_SEQ(factory, XTREPORTSGR,    "\E[",  Ps, 0, "#|");
    //
    REGISTER_SEQ(factory, XTSMGRAPHICS,   "\E[?", Ps, 0, "S");
    REGISTER_SEQ(factory, XTRESTORE,      "\E[?", Ps, 0, "r");
    REGISTER_SEQ(factory, XTSAVE,         "\E[?", Ps, 0, "s");
    REGISTER_SEQ(factory, XTHIMOUSE,      "\E[",  Ps, 0, "T");
    REGISTER_SEQ(factory, XTPUSHSGR,      "\E[",  Ps, 0, "p");
    REGISTER_SEQ(factory, XTPUSHSGR,      "\E[",  No, 0, "#{");
    REGISTER_SEQ(factory, XTPOPSGR,       "\E[",  Ps, 0, "q");
    REGISTER_SEQ(factory, XTPOPSGR,       "\E[",  No, 0, "#}");
    REGISTER_SEQ(factory, XTCHECKSUM,     "\E[",  Ps, 0, "#y");
}
