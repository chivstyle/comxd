/*!
// (c) 2021 chiv
//
*/
#include "VT420Charset.h"
#include "connimpls/vt300/VT320Charset.h"

uint32_t VT420_RemapCharacter(uint32_t uc, int cs, int extended_cs)
{
	return VT320_RemapCharacter(uc, cs, extended_cs);
}
