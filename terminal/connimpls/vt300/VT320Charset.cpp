/*!
// (c) 2021 chiv
//
*/
#include "VT320Charset.h"
#include "connimpls/vt200/VT220Charset.h"

uint32_t VT320_RemapL(uint32_t uc, int cs)
{
    switch (cs) {
    case CS_PORTUGUESE:
        switch (uc) {
        case 0x23:
            return L'#';
        case 0x40:
            return L'@';
        case 0x5b:
            return L'Ã';
        case 0x5c:
            return L'Ç';
        case 0x5d:
            return L'Õ';
        case 0x5e:
            return L'^';
        case 0x5f:
            return L'_';
        case 0x60:
            return L'`';
        case 0x7b:
            return L'ã';
        case 0x7c:
            return L'ç';
        case 0x7d:
            return L'õ';
        case 0x7e:
            return L'~';
        }
        break;
    case CS_DANISH_VT320:
        cs = CS_DANISH;
    default:
        break;
    }
    return VT220_RemapL(uc, cs);
}
uint32_t VT320_RemapR(uint32_t uc, int cs)
{
    switch (cs) {
    case CS_USER_PREFERED_SUPPLEMENTAL:
        // we do not process this cs, it was remapped to ISO Latin-1
        break;
    case CS_DEC_SUPPLEMENTAL_VT320:
        cs = CS_DEC_SUPPLEMENTAL;
    default:
        break;
    }
    return VT220_RemapR(uc, cs);
}

uint32_t VT320_RemapCharacter(uint32_t uc, int cs, int extended_cs)
{
    if (uc < 0x80)
        return VT320_RemapL(uc, cs);
    else if (uc < 0x100)
        return VT320_RemapR(uc, extended_cs);
    else
        return uc;
}
