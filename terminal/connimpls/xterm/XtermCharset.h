/*!
// (c) 2021 chiv
//
*/
#pragma once

#include <stdint.h>
#include "connimpls/Charset.h"

enum XtermCharset {
    XTERM_FOO = XTERM_CS_BEGIN,
    XTERM_CS_MAX
};
static_assert(XTERM_CS_MAX < XTERM_CS_END, "please define XTERM_CS_COUNT big enough");

uint32_t Xterm_RemapCharacter(uint32_t uc, int cs, int extended_cs);
