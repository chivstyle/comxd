//
// (c) 2020 chiv
//
#include "ColorTable.h"

using namespace Upp;

VTColorTable::VTColorTable()
    : mFallbackColor(Upp::White())
{
    SetToDefault();
    // build palette
    for (int k = 0; k < 256; ++k) {
        SetColor(kColorId_Max + k, Upp::Color(rand() % 255, rand() % 255, rand() % 255));
    }
}

VTColorTable::~VTColorTable()
{
}

void VTColorTable::SetToDefault()
{
    mTbl[kColorId_Texts] = Color(223, 223, 223);
    mTbl[kColorId_Paper] = Color(30, 30, 30);
    mTbl[kColorId_Red] = Color(200, 47, 47);
    mTbl[kColorId_Green] = Color(35, 209, 139);
    mTbl[kColorId_Blue] = Color(59, 142, 234);
    mTbl[kColorId_Black] = Color(80, 80, 80);
    mTbl[kColorId_White] = Color(223, 223, 223);
    mTbl[kColorId_Magenta] = Color(188, 63, 188);
    mTbl[kColorId_Cyan] = Color(16, 223, 223);
    mTbl[kColorId_Yellow] = Color(225, 225, 15);
}

int VTColorTable::FindNearestColorId(const Upp::Color& color)
{
    int c_idx = kColorId_Max;
    int min_d = 255 * 3;
    for (int k = 0; k < 256; ++k) {
        Upp::Color cr = GetColor(kColorId_Max + k);
        int d = abs(cr.GetR() - color.GetR()) + abs(cr.GetG() - color.GetG()) + abs(cr.GetB() - color.GetB());
        if (d < min_d) {
            min_d = d;
            c_idx = kColorId_Max + k;
        }
    }
    return c_idx;
}
int VTColorTable::FindNearestColorId(int index)
{
    return kColorId_Max + (abs(index) % 255);
}

void VTColorTable::SetFallbackColor(const Upp::Color& color)
{
    mFallbackColor = color;
}

const Color& VTColorTable::GetIndexColor(int color_id)
{
    auto it = mTbl.find(kColorId_Max + color_id);
    if (it != mTbl.end()) {
        return it->second;
    }
    return mFallbackColor;
}

const Color& VTColorTable::GetColor(int color_id)
{
    auto it = mTbl.find(color_id);
    if (it != mTbl.end()) {
        return it->second;
    }
    return mFallbackColor;
}
