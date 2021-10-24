/*!
// (c) 2021 chiv
*/
#include "VT420ControlSeq.h"

#define kESC "\033"
#define kCSI "\033["
#define kDEC "\033[?"
#define kSS3 "\033O"

void AddVT420ControlSeqs(ControlSeqFactory* factory)
{
    REGISTER_SEQ(factory, ANSI_CONFORMANCE_LEVEL_1, "\033 ", No, 0, "L");
    REGISTER_SEQ(factory, ANSI_CONFORMANCE_LEVEL_2, "\033 ", No, 0, "M");
    REGISTER_SEQ(factory, ANSI_CONFORMANCE_LEVEL_3, "\033 ", No, 0, "N");
    //
    REGISTER_SEQ(factory, DECSLPP, kCSI, Pn, 1, "t");
    REGISTER_SEQ(factory, DECSLRM, kCSI, Pn, 2, "s");
    //
    REGISTER_SEQ(factory, DECCRA, kCSI, Ps, 0, "$v");
    REGISTER_SEQ(factory, DECFRA, kCSI, Ps, 0, "$z");
    REGISTER_SEQ(factory, DECSERA, kCSI, Ps, 0, "${");
    REGISTER_SEQ(factory, DECSACE, kCSI, Ps, 0, "$x");
    REGISTER_SEQ(factory, DECCARA, kCSI, Ps, 0, "$r");
    REGISTER_SEQ(factory, DECRARA, kCSI, Ps, 0, "$t");
    //
    REGISTER_SEQ(factory, DECELF, kCSI, Ps, 0, "+q");
    REGISTER_SEQ(factory, DECLFKC, kCSI, Ps, 0, "*}");
    REGISTER_SEQ(factory, DECSMKR, kCSI, Ps, 0, "+r");
    //
    REGISTER_SEQ(factory, DECDC, kCSI, Pn, 1, ",~");
    REGISTER_SEQ(factory, DECIC, kCSI, Pn, 1, ",}");
    //
    REGISTER_SEQ(factory, DECBI, "\0336", No, 0, "");
    REGISTER_SEQ(factory, DECFI, "\0339", No, 0, "");
    //
    REGISTER_SEQ(factory, DECSNLS, kCSI, Pn, 1, "*|");
    //
    REGISTER_SEQ(factory, TertiaryDA, "\033[=", Pn, 1, "c");
    //
    REGISTER_SEQ(factory, DECRQCRA, kCSI, Ps, 0, "*y");
    REGISTER_SEQ(factory, DECRQDE, kCSI, No, 0, "\"v");
    //
    REGISTER_SEQ(factory, DECES, kCSI, No, 0, "&x");
    REGISTER_SEQ(factory, DECINVM, kCSI, Pn, 1, "*z");
}
