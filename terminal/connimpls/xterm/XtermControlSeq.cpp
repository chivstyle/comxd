/*!
// (c) 2021 chiv
*/
#include "XtermControlSeq.h"

#define kESC "\x1b"
#define kCSI "\x1b["
#define kDEC "\x1b[?"
#define kSS3 "\x1bO"

void AddXtermControlSeqs(ControlSeqFactory* factory)
{
    REGISTER_SEQ(factory, XTWINOPS, kCSI, Ps, 0, "t");
}
