/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "connimpls/Charset.h"
#include <stdint.h>

namespace xvt {
enum VT220Charset {
    CS_DEC_SUPPLEMENTAL = VT200_CS_BEGIN,
    CS_ASCII,
    CS_DEC_SPECIAL_GRAPHICS,
    CS_BRITISH,
    CS_DANISH,
    CS_DUTCH,
    CS_FINNISH,
    CS_FLEMISH,
    CS_FRENCH,
    CS_FRENCH_CANADIAN,
    CS_GERMAN,
    CS_ITALIAN,
    CS_NORWEGIAN,
    CS_SPANISH,
    CS_SWEDISH,
    CS_SWISS,
    VT220_CS_MAX
};
static_assert(VT220_CS_MAX < VT200_CS_END, "please define VT200_CS_COUNT big enough");

uint32_t VT220_RemapL(uint32_t uc, int cs);
uint32_t VT220_RemapR(uint32_t uc, int cs);
uint32_t VT220_RemapCharacter(uint32_t uc, int cs, int extended_cs);
}
