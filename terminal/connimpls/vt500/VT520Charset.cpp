/*!
// (c) 2021 chiv
//
*/
#include "VT520Charset.h"
#include "connimpls/vt400/VT420Charset.h"

uint32_t VT520_RemapCharacter(uint32_t uc, int cs, int extended_cs)
{
	return VT420_RemapCharacter(uc, cs, extended_cs);
}
