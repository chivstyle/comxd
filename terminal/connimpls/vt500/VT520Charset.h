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
    CS_HEBREW = VT500_CS_BEGIN,
    CS_GREEK,
    CS_TURKISH,
    CS_SCS,
    CS_RUSSIAN,
    CS_ISO_LATIN2_SUPPLEMENTAL,
    CS_ISO_GREEK_SUPPLEMENTAL,
    CS_ISO_HEBREW_SUPPLEMENTAL,
    CS_ISO_LATIN_CYRILLIC,
    CS_ISO_LATIN5_SUPPLEMENTAL,
    VT520_CS_MAX
};
static_assert(VT520_CS_MAX < VT500_CS_END, "please define VT500_CS_COUNT big enough");

uint32_t VT520_RemapL(uint32_t uc, int cs);
uint32_t VT520_RemapR(uint32_t uc, int cs);
uint32_t VT520_RemapCharacter(uint32_t uc, int cs, int extended_cs);
