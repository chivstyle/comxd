//
// (c) 2020 chiv
//
#include "VTtypes.h"
#include "ColorTable.h"
// default style
VTStyle::VTStyle()
    : FontStyle(eVisible)
    , FgColorId(VTColorTable::kColorId_Texts)
    , BgColorId(VTColorTable::kColorId_Paper)
{
}

VTStyle::VTStyle(uint16_t font_style, uint16_t fg_colorid, uint16_t bg_colorid)
{
    FontStyle = font_style;
    FgColorId = fg_colorid;
    BgColorId = bg_colorid;
}