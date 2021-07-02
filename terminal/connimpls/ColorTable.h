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
    //
    enum ColorId {
        kColorId_Texts,
        kColorId_Paper,
        kColorId_Red,
        kColorId_Green,
        kColorId_Blue,
        kColorId_Black,
        kColorId_White,
        kColorId_Magenta,
        kColorId_Cyan,
        kColorId_Yellow,
        //
        kColorId_Max
    };
    //
    void Swap(const ColorId& color_id1, const ColorId& color_id2)
    {
        std::swap(mTbl[color_id1], mTbl[color_id2]);
    }
    //
    const Upp::Color& GetColor(int color_id);
    const Upp::Color& GetIndexColor(int index);
    
    void SetColor(int color_id, const Upp::Color& color)
    {
        mTbl[color_id] = color;
    }

private:
    std::unordered_map<int, Upp::Color> mTbl;
    //
    Upp::Color mFallbackColor;
};


#endif
