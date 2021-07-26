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
    REGISTER_SEQ(factory, DECKBD,  "\E[", Ps, 2, " }");
    REGISTER_SEQ(factory, DECLTOD, "\E[", Ps, 2, ",p");
    REGISTER_SEQ(factory, DECPCTERM, "\E[?", Ps, 2, "r");
    REGISTER_SEQ(factory, DECPKA, "\E[", Ps, 1, "+z");
    REGISTER_SEQ(factory, DECPS, "\E[", Ps, 3, ",~");
    REGISTER_SEQ(factory, DECRQKD, "\E[", Ps, 2, ",w");
    REGISTER_SEQ(factory, DECRQKT, "\E[", Ps, 1, ",u");
    REGISTER_SEQ(factory, DECRQM_ANSI, "\E[", Ps, 1, "$p");
    REGISTER_SEQ(factory, DECRQM_DECP, "\E[?", Ps, 1, "$p");
    REGISTER_SEQ(factory, DECRQPKFM, "\E[+x", No, 0, "");
    REGISTER_SEQ(factory, DECCTR, "\E[2;", Ps, 1, "$u");
    REGISTER_SEQ(factory, DECSCP, "\E[", Ps, 2, "*u");
    REGISTER_SEQ(factory, DECSCS, "\E[", Ps, 2, "*r");
    REGISTER_SEQ(factory, DECSCUSR, "\E[", Ps, 1, " q");
    REGISTER_SEQ(factory, DECSDDT, "\E[", Ps, 1, "$q");
    REGISTER_SEQ(factory, DECSDPT, "\E[", Ps, 1, ")q");
    REGISTER_SEQ(factory, DECSEST, "\E[", Ps, 1, "-r");
    REGISTER_SEQ(factory, DECSFC, "\E[", Ps, 4, "*s");
    REGISTER_SEQ(factory, DECSKCV, "\E[", Ps, 1, " r");
    REGISTER_SEQ(factory, DECSLCK, "\E[", Ps, 1, " v");
    REGISTER_SEQ(factory, DECSMBV, "\E[", Ps, 1, " u");
    REGISTER_SEQ(factory, DECSPMA, "\E[", Pn, 4, ",x");
    REGISTER_SEQ(factory, DECSPP, "\E[", Ps, 4, "+w");
    REGISTER_SEQ(factory, DECSPPCS, "\E[", Pn, 1, "*p");
    REGISTER_SEQ(factory, DECSPRTT, "\E[", Ps, 1, "$s");
    REGISTER_SEQ(factory, DECSR, "\E[", Ps, 1, "+p");
    REGISTER_SEQ(factory, DECSRC, "\E[", Ps, 1, "+q");
    REGISTER_SEQ(factory, DECSSCLS, "\E[", Ps, 1, " p");
    REGISTER_SEQ(factory, DECSSL, "\E[", Ps, 1, "p");
    REGISTER_SEQ(factory, DECSSL, "\E[?5W", No, 0, "");
    REGISTER_SEQ(factory, DECSTGLT, "\E[", Ps, 1, "){");
    REGISTER_SEQ(factory, DECSTRL, "\E[", Ps, 2, "\"u");
    REGISTER_SEQ(factory, DECSWBV, "\E[", Ps, 1, " t");
    REGISTER_SEQ(factory, DECSZS, "\E[", Ps, 1, ",{");
    REGISTER_SEQ(factory, DECTID, "\E[", Ps, 1, ",q");
    REGISTER_SEQ(factory, DECTME, "\E[", Ps, 1, " ~");
    REGISTER_SEQ(factory, DECUS, "\E[", Ps, 1, " ,y");
}
