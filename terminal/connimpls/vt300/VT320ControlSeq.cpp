/*!
// (c) 2021 chiv
*/
#include "VT320ControlSeq.h"

#define kCSI "\x1b["
#define kDEC "\x1b[?"
#define kSS3 "\x1bO"

void AddVT320ControlSeqs(ControlSeqFactory* factory)
{
    REGISTER_SEQ(factory, DECSASD, kCSI, Ps, 0, "$}");
    REGISTER_SEQ(factory, DECSSDT, kCSI, Ps, 0, "$~");
    REGISTER_SEQ(factory, DECRQTSR, kCSI, Ps, 0, "$u");
    REGISTER_SEQ(factory, DECRQPSR, kCSI, Ps, 0, "$w");
    REGISTER_SEQ(factory, DECRQM, kDEC, Ps, 0, "$p");
    REGISTER_SEQ(factory, ANSIRQM, kCSI, Ps, 0, "$p");
    REGISTER_SEQ(factory, DECRPM, kCSI, Ps, 0, "$y");
    REGISTER_SEQ(factory, DECRQUPSS, kCSI, No, 0, "&u");
}
