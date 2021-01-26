/*!
// (C) 2021 chiv
//
*/
#include "VT100Charset.h"

uint32_t VT100_RemapCharacter(uint32_t uc, int type)
{
    const uint32_t cs[] = {
        0x02666,
        0x02592,
        0x02192,
        0x00192,
        0x0027c,
        0x0091e,
        0x000b0,
        0x000b1,
        0x0019e,
        0x02193,
        0x02518,
        0x02510,
        0x0250c,
        0x02514,
        0x0253c,
        0x023ba,
        0x023bb,
        0x02500,
        0x023bc,
        0x023bd,
        0x0251c,
        0x02524,
        0x02534,
        0x0252c,
        0x02502,
        0x02264,
        0x02265,
        0x003c0,
        0x02260,
        0x000a3,
        0x000b7
    };
    switch (type) {
    case CS_UK: if (uc == '#') return 0x000a3; else break;
    case CS_LINE_DRAWING: if (uc >= '`' && uc <= '~') return cs[uc - '`']; else break;
    case CS_US:
    case CS_ROM:
    case CS_ROM_SPECIAL:
    default: break;
    }
    return uc;
}
