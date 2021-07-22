/*!
// (c) 2021 chiv
*/
#include "VT520ControlSeq.h"

#define kESC "\E"
#define kCSI "\E["
#define kDEC "\E[?"
#define kSS3 "\EO"

void AddVT520ControlSeqs(ControlSeqFactory* factory)
{
    REGISTER_SEQ(factory, DDD1, "\E)1", No, 0, "");
    REGISTER_SEQ(factory, DDD2, "\E)B", No, 0, "");
    REGISTER_SEQ(factory, DDD3, "\E(1", No, 0, "");
    //
    REGISTER_SEQ(factory, DECAC,    "\E[", Ps, 3, ",|");
    REGISTER_SEQ(factory, DECARR,   "\E[", Ps, 1, "-p");
    REGISTER_SEQ(factory, DECATC,   "\E[", Ps, 3, ",}");
    REGISTER_SEQ(factory, DECCKSR,  "\E[", Ps, 0, "n");
    REGISTER_SEQ(factory, DECCRTST, "\E[", Ps, 1, "-q");
    //
    REGISTER_SEQ(factory, DECDHLT, "\E#3", No, 0, "");
    REGISTER_SEQ(factory, DECDHLB, "\E#4", No, 0, "");
    //
    REGISTER_SEQ(factory, DECDLDA, "\E[", Ps, 1, ",z");
    REGISTER_SEQ(factory, DECELF,  "\E[", Ps, 0, "+q");
    REGISTER_SEQ(factory, DECKBD,  "\E[", Ps, 2, " }");
    REGISTER_SEQ(factory, DECLFKC, "\E[", Ps, 0, "*}");
    REGISTER_SEQ(factory, DECLTOD, "\E[", Ps, 2, ",p");
    REGISTER_SEQ(factory, DECPCTERM, "\E[?", Ps, 2, "r");
    REGISTER_SEQ(factory, DECPKA, "\E[", Ps, 1, "+z");
    REGISTER_SEQ(factory, DECPS, "\E[", Ps, 3, ",~");
    REGISTER_SEQ(factory, DECRQCRA, "\E[", Ps, 6, "*y");
    REGISTER_SEQ(factory, DECRQDE, "\E[", No, 0, "\"v");
    REGISTRE_SEQ(factory, DECRQKD, "\E[", Ps, 2, ",w");
    REGISTER_SEQ(factory, DECRQKT, "\E[", Ps, 1, ",u");
    REGISTER_SEQ(factory, DECRQM_ANSI, "\E[", Ps, 1, "$p");
    REGISTER_SEQ(factory, DECRQM_DECP, "\E[?", Ps, 1, "$p");
}
