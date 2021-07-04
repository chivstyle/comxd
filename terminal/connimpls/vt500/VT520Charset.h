/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "connimpls/Charset.h"
#include <stdint.h>

enum VT520Charset {
    // There's a ISO Latin-1 Supplemental in vt520, but we ignore it, why ?
    // We use unicode to render characters, 0x80-0xff was remapped to Latin1 already.
    VT520_CS_BEGIN = VT400_CS_BEGIN,
    VT520_CS_MAX
};
static_assert(VT520_CS_MAX < VT400_CS_END, "please define VT400_CS_COUNT big enough");

uint32_t VT520_RemapCharacter(uint32_t uc, int cs, int extended_cs);
