/*!
// (c) 2021 chiv
//
*/
#include "VT420Charset.h"
#include "connimpls/vt300/VT320Charset.h"

namespace xvt {

uint32_t VT420_RemapL(uint32_t uc, int cs)
{
    return VT320_RemapL(uc, cs);
}
uint32_t VT420_RemapR(uint32_t uc, int cs)
{
    return VT320_RemapR(uc, cs);
}
uint32_t VT420_RemapCharacter(uint32_t uc, int cs, int extended_cs)
{
    return VT320_RemapCharacter(uc, cs, extended_cs);
}
}
