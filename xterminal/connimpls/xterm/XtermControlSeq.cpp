/*!
// (c) 2021 chiv
*/
#include "XtermControlSeq.h"

// HELP
// CSI           "\033["
// OSC           "\033]
// DEC private   "\033[?"
//
void AddXtermControlSeqs(ControlSeqFactory* factory)
{
    REGISTER_SEQ(factory, XTWINOPS, "\033[", Ps, 0, "t");
    REGISTER_SEQ(factory, XTOSC, "\033]", Gs, 0, "\x07");
    REGISTER_SEQ(factory, XTRMTITLE, "\033[>", Ps, 0, "T");
    REGISTER_SEQ(factory, XTSMTITLE, "\033[>", Ps, 0, "t");
    REGISTER_SEQ(factory, XTMODKEYS, "\033[>", Ps, 0, "m");
    REGISTER_SEQ(factory, XTDISABLEMODOPTS, "\033[>", Ps, 0, "n");
    REGISTER_SEQ(factory, XTSMPOINTER, "\033[>", Ps, 0, "p");
    REGISTER_SEQ(factory, XTVERSION, "\033[>", Ps, 0, "q");
    REGISTER_SEQ(factory, XTPUSHCOLORS, "\033[", Ps, 0, "#P");
    REGISTER_SEQ(factory, XTPOPCOLORS, "\033[", Ps, 0, "#Q");
    // The 2 seqs below are responses
    REGISTER_SEQ(factory, XTREPORTCOLORS, "\033[", No, 0, "#R");
    REGISTER_SEQ(factory, XTREPORTSGR, "\033[", Ps, 0, "#|");
    //
    REGISTER_SEQ(factory, XTSMGRAPHICS, "\033[?", Ps, 0, "S");
    REGISTER_SEQ(factory, XTRESTORE, "\033[?", Ps, 0, "r");
    REGISTER_SEQ(factory, XTSAVE, "\033[?", Ps, 0, "s");
    // For convenience, we do not use strict parameter conditions, so
    // "\033[1T", "\033[1;2;3;4;5T", .etc were parsed as A single seq, the ECMA-048
    // defines it to SD, if there are three Ps in the parameter, you should
    // treat the seq as XTHIMOUSE. please override ProcessSD to implement this feature.
#if 0
    REGISTER_SEQ(factory, XTHIMOUSE, "\033[", Ps, 0, "T");
#endif
    REGISTER_SEQ(factory, XTPUSHSGR, "\033[", Ps, 0, "p");
    REGISTER_SEQ(factory, XTPUSHSGR, "\033[", No, 0, "#{");
    REGISTER_SEQ(factory, XTPOPSGR, "\033[", Ps, 0, "q");
    REGISTER_SEQ(factory, XTPOPSGR, "\033[", No, 0, "#}");
    REGISTER_SEQ(factory, XTCHECKSUM, "\033[", Ps, 0, "#y");
    //
    REGISTER_SEQ(factory, ANSI_CONFORMANCE_LEVEL1, "\033 L", No, 0, "");
    REGISTER_SEQ(factory, ANSI_CONFORMANCE_LEVEL2, "\033 M", No, 0, "");
    REGISTER_SEQ(factory, ANSI_CONFORMANCE_LEVEL3, "\033 N", No, 0, "");
    // Ignore all these seqs
    // These seqs were responsed to shell, we still list them here to prevent the user
    // send them to me. If we do not add them to factory, the VT will enter undefined state.
    // Don't worry, it's okay to use the VT normally.
    REGISTER_SEQ(factory, XTIGNORE, "\033[I", Gn, 0, "");
    REGISTER_SEQ(factory, XTIGNORE, "\033[O", Gn, 0, "");
    REGISTER_SEQ(factory, XTIGNORE, "\033[<", Ps, 0, "M");
    REGISTER_SEQ(factory, XTIGNORE, "\033[<", Ps, 0, "m");
    REGISTER_SEQ(factory, XTIGNORE, "\033[M", Gn, 3, "");
}
