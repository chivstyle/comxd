/*!
// (c) 2021 chiv
*/
#include "VT220ControlSeq.h"
#include "connimpls/vt100/VT100ControlSeq.h"

#define kCSI "\033["
#define kDEC "\033[?"
#define kSS3 "\033O"

namespace xvt {

void AddVT220ControlSeqs(ControlSeqFactory* factory)
{
    REGISTER_SEQ(factory, DECSCL, kCSI, Ps, 0, "\"p");
    // supplemental to vt100
    REGISTER_SEQ(factory, G0_CS, "\033(", Gn, 1, "");
    REGISTER_SEQ(factory, G1_CS, "\033)", Gn, 1, "");
    REGISTER_SEQ(factory, G2_CS, "\033*", Gn, 1, "");
    REGISTER_SEQ(factory, G3_CS, "\033+", Gn, 1, "");
    REGISTER_SEQ(factory, G0_CS, "\033(", Gn, 2, "");
    REGISTER_SEQ(factory, G1_CS, "\033)", Gn, 2, "");
    REGISTER_SEQ(factory, G2_CS, "\033*", Gn, 2, "");
    REGISTER_SEQ(factory, G3_CS, "\033+", Gn, 2, "");
    REGISTER_SEQ(factory, G0_CS, "\033(", Gn, 3, "");
    REGISTER_SEQ(factory, G1_CS, "\033)", Gn, 3, "");
    REGISTER_SEQ(factory, G2_CS, "\033*", Gn, 3, "");
    REGISTER_SEQ(factory, G3_CS, "\033+", Gn, 3, "");
    //
    REGISTER_SEQ(factory, S7C1T, "\033 F", No, 0, "");
    REGISTER_SEQ(factory, S8C1T, "\033 G", No, 0, "");
    REGISTER_SEQ(factory, DECSCA, kCSI, Ps, 0, "\"q");
    REGISTER_SEQ(factory, DECSEL, kDEC, Ps, 0, "K");
    REGISTER_SEQ(factory, DECSED, kDEC, Ps, 0, "J");
    // capture all printing sequences
    REGISTER_SEQ(factory, PRINTING, kCSI, Ps, 0, "i");
    REGISTER_SEQ(factory, PRINTING, kDEC, Ps, 0, "i");
    REGISTER_SEQ(factory, SecondaryDA, "\033[>", Ps, 0, "c");
    REGISTER_SEQ(factory, DECSTR, kCSI, No, 0, "!p");
}
}
