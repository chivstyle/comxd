/*!
// (c) 2021 chiv
//
*/
#pragma once

#include <stdint.h>
#include "connimpls/Charset.h"

enum VT320Charset {
    VT320_CS_BEGIN = VT300_CS_BEGIN,
    VT320_CS_MAX
};
static_assert(VT320_CS_MAX < VT300_CS_END, "please define VT300_CS_COUNT big enough");

uint32_t VT320_RemapCharacter(uint32_t uc, int cs, int extended_cs);