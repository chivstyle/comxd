//
// (C) 2020 chiv
//
#ifndef _terminal_ColorTable_h_
#define _terminal_ColorTable_h_
//
#include <CtrlLib/CtrlLib.h>
#include <unordered_map>
// COLOR TABLE
class VTColorTable {
public:
    VTColorTable();
    virtual ~VTColorTable();
    //
    void SetFallbackColor(const Upp::Color& color);
    // colors
    // Layout [0-16] ANSI Colors, others were remainder of 88 or 256 colors
    enum ColorId {
        // basic colors
        kColorId_None = 1024,
        kColorId_Texts = 1025,
        kColorId_Paper = 1026,
        kColorId_Red = 0,
        kColorId_Green = 1,
        kColorId_Blue = 2,
        kColorId_Black = 3,
        kColorId_White = 4,
        kColorId_Magenta = 5,
        kColorId_Cyan = 6,
        kColorId_Yellow = 7,
        kColorId_Max
    };
    //
    void SetToDefault();
    //
    int FindNearestColorId(const Upp::Color& color);
    int FindNearestColorId(int idx);
    //
    void Swap(const ColorId& color_id1, const ColorId& color_id2)
    {
        std::swap(mTbl[color_id1], mTbl[color_id2]);
    }
    //
    const Upp::Color& GetColor(int color_id);

    void SetColor(int color_id, const Upp::Color& color)
    {
        mTbl[color_id] = color;
    }
    void ResetColor(int color_id);

private:
    std::unordered_map<int, Upp::Color> mTbl;
    //
    Upp::Color mFallbackColor;
};

#endif
