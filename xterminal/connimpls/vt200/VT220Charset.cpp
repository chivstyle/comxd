/*!
// (c) 2021 chiv
//
*/
#include "VT220Charset.h"
#include "connimpls/vt100/VT100Charset.h"

static const uint32_t kDecSupplementalGs[] = {
    // C1 CODES
    '?', '?', '?', '?', '?', '?', '?', '?',
    '?', '?', '?', '?', '?', '?', '?', '?',
    '?', '?', '?', '?', '?', '?', '?', '?',
    '?', '?', '?', '?', '?', '?', '?', '?',
    /*
    L' ', L'¡', L'¢', L'£', L' ', L'¥', L' ', L'§',
    L'¤', L'©', L'ª', L'«', L' ', L' ', L' ', L' ',
    L'°', L'±', L'²', L'³', L' ', L'µ', L'¶', L'·',
    L' ', L'¹', L'º', L'»', L'¼', L'½', L' ', L'¿',
    L' ', L'Ñ', L'Ò', L'Ó', L'Ô', L'Õ', L'Ö', L'Œ',
    L'Ø', L'Ù', L'Ú', L'Û', L'Ü', L'Ÿ', L' ', L'ß',
    L'à', L'á', L'â', L'ã', L'ä', L'å', L'æ', L'ç',
    L'è', L'é', L'ê', L'ë', L'ì', L'í', L'î', L'ï',
    L' ', L'ñ', L'ò', L'ó', L'ô', L'õ', L'ö', L'œ',
    L'ø', L'ù', L'ú', L'û', L'ü', L'ÿ', L' ', L' '
    */
    0x20, 0xa1, 0xa2, 0xa3, 0x20, 0xa5, 0x20, 0xa7,
    0xa4, 0xa9, 0xaa, 0xab, 0x20, 0x20, 0x20, 0x20,
    0xb0, 0xb1, 0xb2, 0xb3, 0x20, 0xb5, 0xb6, 0xb7,
    0x20, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0x20, 0xbf,
    0x20, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0x152,
    0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0x178, 0x20, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
    0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0x20, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0x153,
    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xff, 0x20, 0x20
};

uint32_t VT220_RemapR(uint32_t uc, int cs)
{
    switch (cs) {
    case CS_DEC_SUPPLEMENTAL:
        return kDecSupplementalGs[uc - 0x80];
    }
    return uc;
}
uint32_t VT220_RemapL(uint32_t uc, int cs)
{
    switch (cs) {
    case CS_BRITISH:
        if (uc == 0x24)
            return 0xa3;
        else
            break;
    case CS_DEC_SPECIAL_GRAPHICS:
        return VT100_RemapCharacter(uc, CS_LINE_DRAWING);
    // vt220, table 2-6
    case CS_DUTCH:
        switch (uc) {
        case 0x23:
            return 0xa3;
        case 0x40:
            return 0xbe;
        case 0x5b:
            return 0x133;
        case 0x5c:
            return 0xbd;
        case 0x5d:
            return 0x7c;
        case 0x7b:
            return 0xa8;
        case 0x7c:
            return 0x66;
        case 0x7d:
            return 0xbc;
        case 0x7e:
            return 0xb4;
        }
        break;
    // vt220, table 2-7
    case CS_FINNISH:
        switch (uc) {
        case 0x5b:
            return 0xc4;
        case 0x5c:
            return 0xd6;
        case 0x5d:
            return 0xc5;
        case 0x5e:
            return 0xdc;
        case 0x60:
            return 0xe9;
        case 0x7b:
            return 0xe4;
        case 0x7c:
            return 0xf6;
        case 0x7d:
            return 0xe5;
        case 0x7e:
            return 0xfc;
        }
        break;
    // vt220, table 2-8
    case CS_FRENCH:
        switch (uc) {
        case 0x23:
            return 0xa3;
        case 0x40:
            return 0xe0;
        case 0x5b:
            return 0xb0;
        case 0x5c:
            return 0xe7;
        case 0x5d:
            return 0xa7;
        case 0x7b:
            return 0xe9;
        case 0x7c:
            return 0xf9;
        case 0x7d:
            return 0xe8;
        case 0x7e:
            return 0xa8;
        }
        break;
    // vt220, table 2-9
    case CS_FRENCH_CANADIAN:
        switch (uc) {
        case 0x40:
            return 0xe0;
        case 0x5b:
            return 0xe2;
        case 0x5c:
            return 0xe7;
        case 0x5d:
            return 0xea;
        case 0x5e:
            return 0xee;
        case 0x60:
            return 0xf4;
        case 0x7b:
            return 0xe9;
        case 0x7c:
            return 0xf9;
        case 0x7d:
            return 0xe8;
        case 0x7e:
            return 0xfb;
        }
        break;
    // vt220, table 2-10
    case CS_GERMAN:
        switch (uc) {
        case 0x40:
            return 0xa7;
        case 0x5b:
            return 0xc4;
        case 0x5c:
            return 0xd6;
        case 0x5d:
            return 0xdc;
        case 0x7b:
            return 0xe4;
        case 0x7c:
            return 0xf6;
        case 0x7d:
            return 0xfc;
        case 0x7e:
            return 0xdf;
        }
        break;
    // vt220, table 2-11
    case CS_ITALIAN:
        switch (uc) {
        case 0x23:
            return 0xa3;
        case 0x40:
            return 0xc4;
        case 0x5b:
            return 0xb0;
        case 0x5c:
            return 0xe7;
        case 0x5d:
            return 0xe9;
        case 0x60:
            return 0xf9;
        case 0x7b:
            return 0xe0;
        case 0x7c:
            return 0xf2;
        case 0x7d:
            return 0xe8;
        case 0x7e:
            return 0xec;
        }
        break;
    // vt220, table 2-12
    case CS_SPANISH:
        switch (uc) {
        case 0x23:
            return 0xa3;
        case 0x40:
            return 0xc4;
        case 0x5b:
            return 0xa1;
        case 0x5c:
            return 0xd1;
        case 0x5d:
            return 0xbf;
        case 0x7b:
            return 0xb0;
        case 0x7c:
            return 0xf1;
        case 0x7d:
            return 0xe7;
        }
        break;
    // vt220, table 2-13
    case CS_SWEDISH:
        switch (uc) {
        case 0x40:
            return 0xc9;
        case 0x5b:
            return 0xc4;
        case 0x5c:
            return 0xd6;
        case 0x5d:
            return 0xc5;
        case 0x5e:
            return 0xdc;
        case 0x60:
            return 0xe9;
        case 0x7b:
            return 0xe4;
        case 0x7c:
            return 0xf6;
        case 0x7d:
            return 0xe5;
        case 0x7e:
            return 0xfc;
        }
        break;
    // vt220, table 2-14
    case CS_SWISS:
        switch (uc) {
        case 0x23:
            return 0xf9;
        case 0x40:
            return 0xe0;
        case 0x5b:
            return 0xe9;
        case 0x5c:
            return 0xe7;
        case 0x5d:
            return 0xea;
        case 0x5e:
            return 0xee;
        case 0x5f:
            return 0xe8;
        case 0x60:
            return 0xf4;
        case 0x7b:
            return 0xe4;
        case 0x7c:
            return 0xf6;
        case 0x7d:
            return 0xfc;
        case 0x7e:
            return 0xfb;
        }
        break;
    default:
        break;
    }
    return uc;
}

uint32_t VT220_RemapCharacter(uint32_t uc, int cs, int extended_cs)
{
    if (uc < 0x80)
        return VT220_RemapL(uc, cs);
    else if (uc < 0x100)
        return VT220_RemapR(uc, extended_cs);
    else
        return uc;
}
