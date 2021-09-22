/*!
// (c) 2021 chiv
*/
#include "AnsiControlSeq.h"

#define kCSI "\x1b["

void AddAnsiControlSeqs(ControlSeqFactory* factory)
{
    REGISTER_SEQ(factory, ANSI_SCP, kCSI, No, 0, "s");
    REGISTER_SEQ(factory, ANSI_RCP, kCSI, No, 0, "u");
}
