/*!
// (c) 2021 chiv
//
*/
#include "VT220Charset.h"
#include "connimpls/vt100/VT100Charset.h"



uint32_t VT220_RemapCharacter(uint32_t uc, int cs, int extended_cs)
{
	switch (cs) {
	case CS_BRITISH: if (uc == 0x24) return 0xa3; else break;
	case CS_DEC_SPECIAL_GRAPHICS: return VT100_RemapCharacter(uc, CS_LINE_DRAWING);
	// vt220, table 2-6
	case CS_DUTCH:
		switch (uc) {
	    case 0x23: return 0xa3;
	    case 0x40: return 0xbe;
	    case 0x5b: return 0x133;
	    case 0x5c: return 0xbd;
	    case 0x5d: return 0x7c;
	    case 0x7b: return 0xa8;
	    case 0x7c: return 0x66;
	    case 0x7d: return 0xbc;
	    case 0x7e: return 0xb4;
		}
		break;
	// vt220, table 2-7
	case CS_FINNISH:
		switch (uc) {
		case 0x5b: return 0xc4;
		case 0x5c: return 0xd6;
		case 0x5d: return 0xc5;
		case 0x5e: return 0xdc;
		case 0x60: return 0xe9;
		case 0x7b: return 0xe4;
	    case 0x7c: return 0xf6;
	    case 0x7d: return 0xe5;
	    case 0x7e: return 0xfc;
		}
		break;
	// vt220, table 2-8
	case CS_FRENCH:
		switch (uc) {
		case 0x23: return 0xa3;
		case 0x40: return 0xe0;
		case 0x5b: return 0xb0;
		case 0x5c: return 0xe7;
		case 0x5d: return 0xa7;
		case 0x7b: return 0xe9;
	    case 0x7c: return 0xf9;
	    case 0x7d: return 0xe8;
	    case 0x7e: return 0xa8;
		}
		break;
	// vt220, table 2-9
	case CS_FRENCH_CANADIAN:
		switch (uc) {
		case 0x40: return 0xe0;
		case 0x5b: return 0xe2;
		case 0x5c: return 0xe7;
		case 0x5d: return 0xea;
		case 0x5e: return 0xee;
		case 0x60: return 0xf4;
		case 0x7b: return 0xe9;
	    case 0x7c: return 0xf9;
	    case 0x7d: return 0xe8;
	    case 0x7e: return 0xfb;
		}
		break;
	// vt220, table 2-10
	case CS_GERMAN:
		switch (uc) {
		case 0x40: return 0xa7;
		case 0x5b: return 0xc4;
		case 0x5c: return 0xd6;
		case 0x5d: return 0xdc;
		case 0x7b: return 0xe4;
	    case 0x7c: return 0xf6;
	    case 0x7d: return 0xfc;
	    case 0x7e: return 0xdf;
		}
		break;
	// vt220, table 2-11
	case CS_ITALIAN:
		switch (uc) {
		case 0x23: return 0xa3;
		case 0x40: return 0xc4;
		case 0x5b: return 0xb0;
		case 0x5c: return 0xe7;
		case 0x5d: return 0xe9;
		case 0x60: return 0xf9;
		case 0x7b: return 0xe0;
	    case 0x7c: return 0xf2;
	    case 0x7d: return 0xe8;
	    case 0x7e: return 0xec;
		}
		break;
	// vt220, table 2-12
	case CS_SPANISH:
		switch (uc) {
		case 0x23: return 0xa3;
		case 0x40: return 0xc4;
		case 0x5b: return 0xa1;
		case 0x5c: return 0xd1;
		case 0x5d: return 0xbf;
		case 0x7b: return 0xb0;
	    case 0x7c: return 0xf1;
	    case 0x7d: return 0xe7;
		}
		break;
	// vt220, table 2-13
	case CS_SWEDISH:
		switch (uc) {
		case 0x40: return 0xc9;
		case 0x5b: return 0xc4;
		case 0x5c: return 0xd6;
		case 0x5d: return 0xc5;
	    case 0x5e: return 0xdc;
	    case 0x60: return 0xe9;
		case 0x7b: return 0xe4;
	    case 0x7c: return 0xf6;
	    case 0x7d: return 0xe5;
	    case 0x7e: return 0xfc;
		}
		break;
	// vt220, table 2-14
	case CS_SWISS:
		switch (uc) {
		case 0x23: return 0xf9;
		case 0x40: return 0xe0;
		case 0x5b: return 0xe9;
		case 0x5c: return 0xe7;
		case 0x5d: return 0xea;
	    case 0x5e: return 0xee;
	    case 0x5f: return 0xe8;
	    case 0x60: return 0xf4;
		case 0x7b: return 0xe4;
	    case 0x7c: return 0xf6;
	    case 0x7d: return 0xfc;
	    case 0x7e: return 0xfb;
		}
		break;
	default:break;
	}
	return uc;
}