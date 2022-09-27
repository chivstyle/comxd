/*!
// (c) 2021 chiv
//
*/
#include "XtermCharset.h"
#include "connimpls/vt500/VT520Charset.h"

namespace xvt {
uint32_t Xterm_RemapCharacter(uint32_t uc, int cs, int extended_cs)
{
    return VT520_RemapCharacter(uc, cs, extended_cs);
}
}
