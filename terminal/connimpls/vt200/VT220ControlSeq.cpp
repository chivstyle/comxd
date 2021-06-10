/*!
// (c) 2021 chiv
*/
#include "VT220ControlSeq.h"
#include "connimpls/vt100/VT100ControlSeq.h"

#define kCSI "\x1b["
#define kDEC "\x1b[?"
#define kSS3 "\x1bO"

void AddVT220ControlSeqs(ControlSeqFactory* factory)
{
    REGISTER_SEQ(factory, DECSCL, kCSI, Ps, 0, "\"p");
    // supplemental to vt100
    REGISTER_SEQ(factory, G0_CS,  "\x1b(", Gn, 1, "");
    REGISTER_SEQ(factory, G1_CS,  "\x1b)", Gn, 1, "");
    REGISTER_SEQ(factory, G2_CS,  "\x1b*", Gn, 1, "");
    REGISTER_SEQ(factory, G3_CS,  "\x1b+", Gn, 1, "");
    REGISTER_SEQ(factory, G0_CS,  "\x1b(", Gn, 2, "");
    REGISTER_SEQ(factory, G1_CS,  "\x1b)", Gn, 2, "");
    REGISTER_SEQ(factory, G2_CS,  "\x1b*", Gn, 2, "");
    REGISTER_SEQ(factory, G3_CS,  "\x1b+", Gn, 2, "");
    REGISTER_SEQ(factory, G0_CS,  "\x1b(", Gn, 3, "");
    REGISTER_SEQ(factory, G1_CS,  "\x1b)", Gn, 3, "");
    REGISTER_SEQ(factory, G2_CS,  "\x1b*", Gn, 3, "");
    REGISTER_SEQ(factory, G3_CS,  "\x1b+", Gn, 3, "");
    //
    REGISTER_SEQ(factory, S7C1T,  "\x1b F", No, 0, "");
    REGISTER_SEQ(factory, S8C1T,  "\x1b G", No, 0, "");
    REGISTER_SEQ(factory, DECSCA,  kCSI, Ps, 0, "\"q");
    REGISTER_SEQ(factory, DECSEL,  kDEC, Ps, 0, "K");
    REGISTER_SEQ(factory, DECSED,  kDEC, Ps, 0, "J");
    // capture all printing sequences
    REGISTER_SEQ(factory, PRINTING,  kCSI, Gs, 0, "i");
    REGISTER_SEQ(factory, SecondaryDA,  "\x1b[>", Ps, 0, "c");
    REGISTER_SEQ(factory, DECSTR, kCSI, No, 0, "!p");
}
