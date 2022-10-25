/*!
// (C) 2021 chiv
//
*/
#include "VT100Charset.h"

uint32_t VT100_RemapCharacter(uint32_t uc, int cs)
{
    static const uint32_t cs_drawing[] = {
        0x20,            // h'5f o'137  _   Blank
        0x025c6,         // h'60 o'140  `   Diamond
        0x02592,         // ..              Checkerboard
        0x02409,         //                 Horizontal tab
        0x0240c,         //                 Form Feed
        0x0240d,         //                 Carriage Return
        0x0240a,         //                 Line Feed
        0x000b0,         //                 Degree Sign
        0x000b1,         //                 Minus Plus
        0x02424,         //                 Newline
        0x0240b,         //                 Vertical tab
        0x02518,         //                 Light Up and Left
        0x02510,         //                 Light Down and Left
        0x0250c,         //                 Light Down and Right
        0x02514,         //                 Light Up and Right
        0x0253c,         //                 Light Vertical and Horizontal
        0x023ba,         //                 Horizontal Scan Line-1
        0x023bb,         //                 Horizontal Scan Line-2
        0x02500,         //                 Light Horizontal / Horizontal Scan Line-5
        0x023bc,         //                 Horizontal Scan Line-7
        0x023bd,         //                 Horizontal Scan Line-9
        0x0251c,         //                 Light Vertical and Right
        0x02524,         //                 Light Vertical and Left
        0x02534,         //                 Light Up and Horizontal
        0x0252c,         //                 Light Down and Horizontal
        0x02502,         //                 Light Vertical
        0x02264,         //                 Less-Than or Equal To
        0x02265,         //                 Greater-Than or Equal To
        0x003c0,         //                 Small Letter Pi
        0x02260,         //                 Not Equal To
        0x000a3,         //                 Pound Sign
        0x000b7          //                 Middle Dot
    };
    switch (cs) {
    case CS_UK:
        if (uc == 0x23)
            return 0x000a3;
        else
            break;
    case CS_LINE_DRAWING:
        if (uc >= 0x5f && uc <= 0x7e)
            return cs_drawing[uc - 0x5f];
        else
            break;
    case CS_US:
    case CS_ROM:
    case CS_ROM_SPECIAL:
    default:
        break;
    }
    return uc;
}
