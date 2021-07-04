/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "connimpls/Charset.h"
#include <stdint.h>

enum VT320Charset {
    CS_DEC_SUPPLEMENTAL_VT320 = VT300_CS_BEGIN,
    CS_USER_PREFERED_SUPPLEMENTAL,
    CS_DANISH_VT320,
    CS_PORTUGUESE,
    VT320_CS_MAX
};
static_assert(VT320_CS_MAX < VT300_CS_END, "please define VT300_CS_COUNT big enough");

uint32_t VT320_RemapL(uint32_t uc, int cs);
uint32_t VT320_RemapR(uint32_t uc, int cs);
uint32_t VT320_RemapCharacter(uint32_t uc, int cs, int extended_cs);
