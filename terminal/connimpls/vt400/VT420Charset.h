/*!
// (c) 2021 chiv
//
*/
#pragma once

#include <stdint.h>
#include "connimpls/Charset.h"

enum VT420Charset {
    // There's a ISO Latin-1 Supplemental in vt420, but we ignore it, why ?
    // We use unicode to render characters, 0x80-0xff was remapped to Latin1 already.
    VT420_CS_BEGIN = VT400_CS_BEGIN,
    VT420_CS_MAX
};
static_assert(VT420_CS_MAX < VT400_CS_END, "please define VT400_CS_COUNT big enough");

uint32_t VT420_RemapCharacter(uint32_t uc, int cs, int extended_cs);
