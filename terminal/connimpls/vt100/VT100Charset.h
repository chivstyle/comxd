/*!
// (c) 2021 chiv
//
*/
#pragma once

#include <stdint.h>
#include "connimpls/Charset.h"

enum VT100Charset {
    CS_US = VT100_CS_BEGIN,
    CS_UK,
    CS_LINE_DRAWING,
    CS_ROM,
    CS_ROM_SPECIAL,
    VT100_CS_MAX
};
static_assert(VT100_CS_MAX < VT100_CS_END, "please define VT100_CS_COUNT big enough");

uint32_t VT100_RemapCharacter(uint32_t uc, int cs);