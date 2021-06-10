/*!
// (c) 2021 chiv
//
*/
#include "VT320Charset.h"
#include "connimpls/vt200/VT220Charset.h"

uint32_t VT320_RemapCharacter(uint32_t uc, int cs, int extended_cs)
{
	return VT220_RemapCharacter(uc, cs, extended_cs);
}
