//
// (c) 2020 chiv
//
#include "ColorTable.h"

using namespace Upp;

VTColorTable::VTColorTable()
	: mFallbackColor(Upp::White())
{
	mTbl[kColorId_Texts] = Upp::Black();
	mTbl[kColorId_Paper] = Upp::LtGray();
	mTbl[kColorId_Red] = Upp::Red();
	mTbl[kColorId_Green] = Upp::Green();
	mTbl[kColorId_Blue] = Upp::Blue();
	mTbl[kColorId_Black] = Upp::Black();
	mTbl[kColorId_White] = Upp::White();
	mTbl[kColorId_Magenta] = Upp::Magenta();
	mTbl[kColorId_Cyan] = Upp::Cyan();
	mTbl[kColorId_Yellow] = Upp::Yellow();
}

VTColorTable::~VTColorTable()
{
}

void VTColorTable::SetFallbackColor(const Upp::Color& color)
{
	mFallbackColor = color;
}

const Color& VTColorTable::GetColor(int color_id)
{
	auto it = mTbl.find(color_id);
	if (it != mTbl.end()) {
		return it->second;
	}
	return mFallbackColor;
}

