/*!
// (c) 2021 chiv
*/
#include "VT520ControlSeq.h"

#define kESC "\033"
#define kCSI "\033["
#define kDEC "\033[?"
#define kSS3 "\033O"

void AddVT520ControlSeqs(ControlSeqFactory* factory)
{
    REGISTER_SEQ(factory, DDD1, "\033)1", No, 0, "");
    REGISTER_SEQ(factory, DDD2, "\033)B", No, 0, "");
    REGISTER_SEQ(factory, DDD3, "\033(1", No, 0, "");
    //
    REGISTER_SEQ(factory, DECAC,    "\033[", Ps, 3, ",|");
    REGISTER_SEQ(factory, DECARR,   "\033[", Ps, 1, "-p");
    REGISTER_SEQ(factory, DECATC,   "\033[", Ps, 3, ",}");
    REGISTER_SEQ(factory, DECCKSR,  "\033[", Ps, 0, "n");
    REGISTER_SEQ(factory, DECCRTST, "\033[", Ps, 1, "-q");
    //
    REGISTER_SEQ(factory, DECDHLT, "\033#3", No, 0, "");
    REGISTER_SEQ(factory, DECDHLB, "\033#4", No, 0, "");
    //
    REGISTER_SEQ(factory, DECDLDA, "\033[", Ps, 1, ",z");
    REGISTER_SEQ(factory, DECKBD,  "\033[", Ps, 2, " }");
    REGISTER_SEQ(factory, DECLTOD, "\033[", Ps, 2, ",p");
    REGISTER_SEQ(factory, DECPCTERM, "\033[?", Ps, 2, "r");
    REGISTER_SEQ(factory, DECPKA, "\033[", Ps, 1, "+z");
    REGISTER_SEQ(factory, DECPS, "\033[", Ps, 3, ",~");
    REGISTER_SEQ(factory, DECRQKD, "\033[", Ps, 2, ",w");
    REGISTER_SEQ(factory, DECRQKT, "\033[", Ps, 1, ",u");
    REGISTER_SEQ(factory, DECRQM_ANSI, "\033[", Ps, 1, "$p");
    REGISTER_SEQ(factory, DECRQM_DECP, "\033[?", Ps, 1, "$p");
    REGISTER_SEQ(factory, DECRQPKFM, "\033[+x", No, 0, "");
    REGISTER_SEQ(factory, DECCTR, "\033[2;", Ps, 1, "$u");
    REGISTER_SEQ(factory, DECSCP, "\033[", Ps, 2, "*u");
    REGISTER_SEQ(factory, DECSCS, "\033[", Ps, 2, "*r");
    REGISTER_SEQ(factory, DECSCUSR, "\033[", Ps, 1, " q");
    REGISTER_SEQ(factory, DECSDDT, "\033[", Ps, 1, "$q");
    REGISTER_SEQ(factory, DECSDPT, "\033[", Ps, 1, ")q");
    REGISTER_SEQ(factory, DECSEST, "\033[", Ps, 1, "-r");
    REGISTER_SEQ(factory, DECSFC, "\033[", Ps, 4, "*s");
    REGISTER_SEQ(factory, DECSKCV, "\033[", Ps, 1, " r");
    REGISTER_SEQ(factory, DECSLCK, "\033[", Ps, 1, " v");
    REGISTER_SEQ(factory, DECSMBV, "\033[", Ps, 1, " u");
    REGISTER_SEQ(factory, DECSPMA, "\033[", Pn, 4, ",x");
    REGISTER_SEQ(factory, DECSPP, "\033[", Ps, 4, "+w");
    REGISTER_SEQ(factory, DECSPPCS, "\033[", Pn, 1, "*p");
    REGISTER_SEQ(factory, DECSPRTT, "\033[", Ps, 1, "$s");
    REGISTER_SEQ(factory, DECSR, "\033[", Ps, 1, "+p");
    REGISTER_SEQ(factory, DECSRC, "\033[", Ps, 1, "+q");
    REGISTER_SEQ(factory, DECSSCLS, "\033[", Ps, 1, " p");
    REGISTER_SEQ(factory, DECSSL, "\033[", Ps, 1, "p");
    REGISTER_SEQ(factory, DECSSL, "\033[?5W", No, 0, "");
    REGISTER_SEQ(factory, DECSTGLT, "\033[", Ps, 1, "){");
    REGISTER_SEQ(factory, DECSTRL, "\033[", Ps, 2, "\"u");
    REGISTER_SEQ(factory, DECSWBV, "\033[", Ps, 1, " t");
    REGISTER_SEQ(factory, DECSZS, "\033[", Ps, 1, ",{");
    REGISTER_SEQ(factory, DECTID, "\033[", Ps, 1, ",q");
    REGISTER_SEQ(factory, DECTME, "\033[", Ps, 1, " ~");
    REGISTER_SEQ(factory, DECUS, "\033[", Ps, 1, " ,y");
}
