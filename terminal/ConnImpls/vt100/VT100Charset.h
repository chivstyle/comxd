/*!
// (c) 2021 chiv
//
*/
#pragma once

#include <stdint.h>
//
enum VT100_CharsetType {
    CS_UK = 0,
    CS_US = 1,
    CS_LINE_DRAWING = 2,
    CS_ROM = 3,
    CS_ROM_SPECIAL = 4
};
uint32_t VT100_RemapCharacter(uint32_t uc, int type);