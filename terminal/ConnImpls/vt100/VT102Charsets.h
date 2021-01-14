/*!
// (c) 2021 chiv
//
*/
#pragma once

#include <CtrlLib/CtrlLib.h>
// UC - UTF-8, ASCII partment
#define G0_ROM G0_US
#define G0_ROM_SPECIAL G0_US
static inline Upp::WString G0_UK(unsigned char uc)
{
    if (uc == '#') return Upp::WString("£");
    else return Upp::WString(uc, 1);
}
static inline Upp::WString G0_US(unsigned char uc)
{
    return Upp::WString(uc, 1);
}
static inline Upp::WString G0_LINE_DRAWING(unsigned char uc)
{
    const char* cs[] = {
		"\xe2\x99\xa6"/* "♦" */,
		"\xe2\x96\x92"/* "▒" */,
		"\xe2\x86\x92"/* "→" */,
		"\xc6\x92"    /* "ƒ" */,
		"\xc9\xbc"    /* "ɼ" */,
		"\xc6\x9e"    /* "ƞ" */,
		"\xc2\xb0"    /* "°" */,
		"\xc2\xb1"    /* "±" */,
		"\xc6\x9e"    /* "ƞ" */,
		"\xe2\x86\x93"/* "↓" */,
		"\xe2\x94\x98"/* "┘" */,
		"\xe2\x94\x90"/* "┐" */,
		"\xe2\x94\x8c"/* "┌" */,
		"\xe2\x94\x94"/* "└" */,
		"\xe2\x94\xbc"/* "┼" */,
		"\xe2\x8e\xba"/* "⎺" */,
		"\xe2\x8e\xbb"/* "⎻" */,
		"\xe2\x94\x80"/* "─" */,
		"\xe2\x8e\xbc"/* "⎼" */,
		"\xe2\x8e\xbd"/* "⎽" */,
		"\xe2\x94\x9c"/* "├" */,
		"\xe2\x94\xa4"/* "┤" */,
		"\xe2\x94\xb4"/* "┴" */,
		"\xe2\x94\xac"/* "┬" */,
		"\xe2\x94\x82"/* "│" */,
		"\xe2\x89\xa4"/* "≤" */,
		"\xe2\x89\xa5"/* "≥" */,
		"\xcf\x80"    /* "π" */,
		"\xe2\x89\xa0"/* "≠" */,
		"\xc2\xa3"    /* "£" */,
		"\xc2\xb7"    /* "·" */
	};
	if (uc >= '`' && uc <= '~') return cs[uc - '`'];
	else return Upp::WString(uc, 1);
}
#define G1_UK            G0_US
#define G1_US            G0_US
#define G1_LINE_DRAWING  G0_LINE_DRAWING
#define G1_ROM           G0_US
#define G1_ROM_SPECIAL   G0_US
