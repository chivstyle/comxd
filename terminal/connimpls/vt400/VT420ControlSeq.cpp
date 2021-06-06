/*!
// (c) 2021 chiv
*/
#include "VT420ControlSeq.h"

#define kESC "\x1b"
#define kCSI "\x1b["
#define kDEC "\x1b[?"
#define kSS3 "\x1bO"

void AddVT420ControlSeqs(ControlSeqFactory* factory)
{
    REGISTER_SEQ(factory, DECINVM, kCSI, Ps, 0, "z");
    REGISTER_SEQ(factory, ANSI_CONFORMANCE_LEVEL_1, "\x1b ", No, 0, "L");
    REGISTER_SEQ(factory, ANSI_CONFORMANCE_LEVEL_2, "\x1b ", No, 0, "M");
    REGISTER_SEQ(factory, ANSI_CONFORMANCE_LEVEL_3, "\x1b ", No, 0, "N");
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
    REGISTER_SEQ(factory, DECBI, "\E6", No, 0, "");
    REGISTER_SEQ(factory, DECFI, "\E9", No, 0, "");
    //
    REGISTER_SEQ(factory, DECSNLS, kCSI, Pn, 1, "*|");
    REGISTER_SEQ(factory, DECSASD, kCSI, Pn, 1, "$}");
    REGISTER_SEQ(factory, DECSSDT, kCSI, Pn, 1, "$~");
    //
    REGISTER_SEQ(factory, TertiaryDA, "\E[=", Pn, 1, "c");
    //
    REGISTER_SEQ(factory, DECRQCRA, kCSI, Ps, 0, "*y");
    REGISTER_SEQ(factory, DECRQDE, kCSI, No, 0, "\"v");
    //
    REGISTER_SEQ(factory, ENABLE_SESSION, kCSI, No, 0, "&x");
}
