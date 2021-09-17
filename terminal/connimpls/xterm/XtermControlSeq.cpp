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
    REGISTER_SEQ(factory, XTWINOPS, "\E[", Ps, 0, "t");
    REGISTER_SEQ(factory, XTOSC, "\E]", Gs, 0, "\x07");
    REGISTER_SEQ(factory, XTRMTITLE, "\E[>", Ps, 0, "T");
    REGISTER_SEQ(factory, XTSMTITLE, "\E[>", Ps, 0, "t");
    REGISTER_SEQ(factory, XTMODKEYS, "\E[>", Ps, 0, "m");
    REGISTER_SEQ(factory, XTDISABLEMODOPTS, "\E[>", Ps, 0, "n");
    REGISTER_SEQ(factory, XTSMPOINTER, "\E[>", Ps, 0, "p");
    REGISTER_SEQ(factory, XTVERSION, "\E[>", Ps, 0, "q");
    REGISTER_SEQ(factory, XTPUSHCOLORS, "\E[", Ps, 0, "#P");
    REGISTER_SEQ(factory, XTPOPCOLORS, "\E[", Ps, 0, "#Q");
    // The 2 seqs below are responses
    REGISTER_SEQ(factory, XTREPORTCOLORS, "\E[", No, 0, "#R");
    REGISTER_SEQ(factory, XTREPORTSGR, "\E[", Ps, 0, "#|");
    //
    REGISTER_SEQ(factory, XTSMGRAPHICS, "\E[?", Ps, 0, "S");
    REGISTER_SEQ(factory, XTRESTORE, "\E[?", Ps, 0, "r");
    REGISTER_SEQ(factory, XTSAVE, "\E[?", Ps, 0, "s");
    // For convenience, we do not use strict parameter conditions, so
    // "\E[1T", "\E[1;2;3;4;5T", .etc were parsed as A single seq, the ECMA-048
    // defines it to SD, if there are three Ps in the parameter, you should
    // treat the seq as XTHIMOUSE. please override ProcessSD to implement this feature.
#if 0
    REGISTER_SEQ(factory, XTHIMOUSE, "\E[", Ps, 0, "T");
#endif
    REGISTER_SEQ(factory, XTPUSHSGR, "\E[", Ps, 0, "p");
    REGISTER_SEQ(factory, XTPUSHSGR, "\E[", No, 0, "#{");
    REGISTER_SEQ(factory, XTPOPSGR, "\E[", Ps, 0, "q");
    REGISTER_SEQ(factory, XTPOPSGR, "\E[", No, 0, "#}");
    REGISTER_SEQ(factory, XTCHECKSUM, "\E[", Ps, 0, "#y");
    //
    REGISTER_SEQ(factory, ANSI_CONFORMANCE_LEVEL1, "\E L", No, 0, "");
    REGISTER_SEQ(factory, ANSI_CONFORMANCE_LEVEL2, "\E M", No, 0, "");
    REGISTER_SEQ(factory, ANSI_CONFORMANCE_LEVEL3, "\E N", No, 0, "");
    // Ignore all these seqs
    // These seqs were responsed to shell, we still list them here to prevent the user
    // send them to me. If we do not add them to factory, the VT will enter undefined state.
    // Don't worry, it's okay to use the VT normally.
    REGISTER_SEQ(factory, XTIGNORE, "\E[I", Gn, 0, "");
    REGISTER_SEQ(factory, XTIGNORE, "\E[O", Gn, 0, "");
    REGISTER_SEQ(factory, XTIGNORE, "\E[<", Ps, 0, "M");
    REGISTER_SEQ(factory, XTIGNORE, "\E[<", Ps, 0, "m");
    REGISTER_SEQ(factory, XTIGNORE, "\E[M", Gn, 3, "");
}