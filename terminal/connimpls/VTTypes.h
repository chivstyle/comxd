//
// (c) 2020 chiv
//
#ifndef _terminal_VTtypes_h_
#define _terminal_VTtypes_h_

#include <CtrlLib/CtrlLib.h>
#include <stdint.h>
#include <string>
#include <vector>

// UPP provides Split to split string, but we do not want to get the splitted string actually,
// so we could do it faster. This routine was designed for VT only.
// Why we do not use strtok, it's not safe. Windows provides strtok_s, Linux provides strtok_r,
// we do not like that way.
// Warning: s should be valid
// Warning: this routine will modify the string s, like strtok
// NOTE: Use Upp::Split, if you want to split string to words.
static inline void SplitString(char* s, size_t s_len, const char* delim, std::function<void(const char*)> func)
{
    size_t p = 0, q = 0; // [p, q) defines a result
    for (; q < s_len && !strchr(delim, s[q]); ++q)
        ;
    while (q < s_len) {
        s[q] = '\0';
        func(s + p);
        p = ++q; // p is the position of next parameter
        // find next separator
        for (; q < s_len && !strchr(delim, s[q]); ++q)
            ;
    }
    // process the left chars
    if (p < s_len) {
        func(s + p);
    }
}
static inline void SplitString(const char* cs, const char* delim, std::function<void(const char*)> func)
{
    static const size_t kCacheSize = 512;
    char _cache[kCacheSize];
    size_t cs_len = strlen(cs);
    if (cs_len == 0) {
        func("");
        return;
    } // return "" for empty string
    if (cs_len < kCacheSize) {
        strcpy(_cache, cs);
        SplitString(_cache, cs_len, delim, func);
    } else {
        char* s = strdup(cs);
        if (s) {
            SplitString(s, cs_len, delim, func);
            free(s);
        }
    }
}
static inline void SplitString(std::string&& s, const char* delim, std::function<void(const char*)> func)
{
    size_t s_len = s.length();
    if (s_len == 0) {
        func("");
        return;
    } // return "" for empty string
    size_t p = 0, q = 0; // [p, q) defines a result
    for (; q < s_len && !strchr(delim, s[q]); ++q)
        ;
    while (q < s_len) {
        s[q] = '\0';
        func(s.data() + p);
        p = ++q;
        for (; q < s_len && !strchr(delim, s[q]); ++q)
            ;
    }
    // process the left chars
    if (p < s_len) {
        func(s.data() + p);
    }
}
//----------------------------------------------------------------------------------------------
struct VTStyle {
    enum FontStyle {
        eBold = 0x01,
        eItalic = 0x02,
        eStrikeout = 0x04,
        eUnderline = 0x08,
        eBlink = 0x10,
        eVisible = 0x20
    };
    uint16_t FontStyle; // lowest 10 bits
    uint16_t FgColorId;
    uint16_t BgColorId;
    VTStyle();
    VTStyle(uint16_t font_style, uint16_t fg_colorid, uint16_t bg_colorid);
};
class VTChar {
public:
    VTChar()
        : VTChar(0)
    {
    }
    VTChar(const uint32_t& c)
    {
        SetCode(c);
        SetStyle(VTStyle()); // default style.
    }
    VTChar(const uint32_t& c, const VTStyle& style)
    {
        SetCode(c);
        SetStyle(style);
    }

    operator const uint32_t()
    {
        return mBits.Code;
    }
    VTChar& operator=(const uint32_t& c)
    {
        SetCode(c);
        return *this;
    }

    void SetCode(const uint32_t& c)
    {
        mBits.Code = c & 0x1fffff;
    }

    void SetStyle(const VTStyle& style)
    {
        mBits.FontStyle = style.FontStyle & 0x3ff;
        mBits.BgColorId = style.BgColorId;
        mBits.FgColorId = style.FgColorId;
    }

    uint32_t Code() const { return mBits.Code; }
    uint16_t FontStyle() const { return mBits.FontStyle; }
    uint16_t FgColorId() const { return mBits.FgColorId; }
    uint16_t BgColorId() const { return mBits.BgColorId; }
    //
    void UseFontStyle(Upp::Font& font, bool& blink, bool& visible) const
    {
        if (mBits.FontStyle & VTStyle::eBold)
            font.Bold();
        else
            font.NoBold();
        if (mBits.FontStyle & VTStyle::eItalic)
            font.Italic();
        else
            font.NoItalic();
        if (mBits.FontStyle & VTStyle::eStrikeout)
            font.Strikeout();
        else
            font.NoStrikeout();
        if (mBits.FontStyle & VTStyle::eUnderline)
            font.Underline();
        else
            font.NoUnderline();
        if (mBits.FontStyle & VTStyle::eBlink)
            blink = true;
        else
            blink = false;
        if (mBits.FontStyle & VTStyle::eVisible)
            visible = true;
        else
            visible = false;
    }
    void UseStyle(Upp::Font& font, uint16_t& fgcolor_id, uint16_t& bgcolor_id,
        bool& blink, bool& visible) const
    {
        UseFontStyle(font, blink, visible);
        fgcolor_id = mBits.FgColorId;
        bgcolor_id = mBits.BgColorId;
    }

private:
    struct
    {
        uint32_t Code : 21; // CODE POINT
        uint32_t FontStyle : 10;
        uint16_t BgColorId;
        uint16_t FgColorId;
    } mBits;
};
/// warning: You should set height manually.
class VTLine : public std::vector<VTChar> {
public:
    VTLine()
    {
        mHeight = 15;
        mHasSuccesiveLines = false;
    }
    explicit VTLine(size_t sz)
        : std::vector<VTChar>(sz)
        , mHeight(15)
        , mHasSuccesiveLines(false)
    {
    }
    VTLine(size_t sz, const VTChar& c)
        : std::vector<VTChar>(sz, c)
        , mHeight(15)
        , mHasSuccesiveLines(false)
    {
    }
    VTLine& SetHeight(int height)
    {
        mHeight = height;
        return *this;
    }
    int GetHeight() const { return mHeight; };
    bool HasSuccessiveLines() const { return mHasSuccesiveLines; }
    void HasSuccessiveLines(bool b) { mHasSuccesiveLines = b; }

private:
    int mHeight;
    bool mHasSuccesiveLines;
};

#endif
