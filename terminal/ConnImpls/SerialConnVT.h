//
// (c) 2020 chiv
//
#ifndef _comxd_ConnVT_h_
#define _comxd_ConnVT_h_

#include "Conn.h"
#include <stdint.h>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <thread>
//----------------------------------------------------------------------------------------------
// UTF8, UTF32
// Helper routines
static inline std::string Utf32ToUtf8(const uint32_t& cp)
{
    std::string out;
    if (cp <= 0x7f) out.push_back((char)cp); // 7 bits
    else if (cp >= 0x80 && cp <= 0x7ff) { // 11 bits, 5, 6
        out.push_back((char)(0xc0 | (cp >> 6))); // high 5 bits
        out.push_back((char)(0x80 | (cp & 0x3f))); // low 6bits
    } else if (cp >= 0x800 && cp <= 0xd7ff || cp >= 0xe000 && cp <= 0xffff) { // 16 bits, 4,6,6
        out.push_back((char)(0xe0 | (cp >> 12)));
        out.push_back((char)(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back((char)(0x80 | (cp & 0x3f)));
    } else if (cp >= 0x10000 && cp <= 0x10ffff) { // 21 bits, 3,6,6,6
        out.push_back((char)(0xf0 | (cp >> 18)));
        out.push_back((char)(0x80 | ((cp >> 12) & 0x3f)));
        out.push_back((char)(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back((char)(0x80 | (cp & 0x3f)));
    } else {
        out.push_back('?'); // not supported code point
    }
    return out;
}
static inline std::vector<uint32_t> Utf8ToUtf32(const std::string& seq, size_t& ep)
{
    std::vector<uint32_t> out;
    size_t p = 0;
    // walk along
    while (p < seq.size()) {
        int flag = seq[p] & 0xf0;
        if (flag == 0xf0) { // 4 bytes
            if (seq.size() - p < 4) break;
            uint32_t bits = (seq[p] & 0x07) << 18; // 3+6+6+6=21bits
            bits |= (seq[p+1] & 0x3f) << 12;
            bits |= (seq[p+2] & 0x3f) << 6;
            bits |= (seq[p+3] & 0x3f);
            out.push_back(bits);
            p += 4;
        } else if ((flag & 0xe0) == 0xe0) { // 3 bytes, 4+6+6=16bits
            if (seq.size() - p < 3) break;
            uint32_t bits = (seq[p] & 0x0f) << 12;
            bits |= (seq[p+1] & 0x3f) << 6;
            bits |= (seq[p+2] & 0x3f);
            out.push_back(bits);
            p += 3;
        } else if ((flag & 0xc0) == 0xc0) { // 2 bytes, 5+6 = 11bits
            if (seq.size() - p < 2) break;
            uint32_t bits = (seq[p] & 0x1f) << 6;
            bits |= (seq[p+1] & 0x3f);
            out.push_back(bits);
            p += 2;
        } else if ((flag & 0x80) == 0x80) { // invalid
            out.push_back('?');
            p++;
        } else {
            out.push_back(seq[p]);
            p++;
        }
    }
    // store the ep
    ep = p;
    //
    return out;
}
//----------------------------------------------------------------------------------------------
// UPP provides Split to split string, but we do not want to get the splitted string actually,
// so we could do it faster. This routine was designed for VT only.
// Why we do not use strtok, it's not safe. Windows provides strtok_s, Linux provides strtok_r,
// we do not like that way.
// Warning: s should be valid
// Warning: this routine will modify the string s, like strtok
// NOTE: Use Upp::Split, if you want to split string to words.
static inline void SplitString(char* s, size_t s_len, char delim, std::function<void(const char*)> func)
{
    size_t p = 0, q = 0; // [p, q) defines a result
    for (; q < s_len && s[q] != delim; ++q);
    while (q < s_len) {
        s[q] = '\0';
        func(s+p);
        p = q+1;
        for (; q < s_len && s[q] != delim; ++q);
    }
    // process the left chars
    if (p < s_len) {
        func(s+p);
    }
}
static inline bool SplitString(const char* cs, char delim, std::function<void(const char*)> func)
{
    static const size_t kCacheSize = 64; char _cache[kCacheSize];
    size_t cs_len = strlen(cs);
    if (cs_len < kCacheSize) {
        strcpy(_cache, cs);
        SplitString(_cache, cs_len, delim, func);
        return true;
    } else {
        char* s = strdup(cs);
        if (s) {
            SplitString(s, cs_len, delim, func);
            free(s);
            return true;
        }
        return false;
    }
}
static inline void SplitString(std::string&& s, char delim, std::function<void(const char*)> func)
{
    size_t s_len = s.length();
    size_t p = 0, q = 0; // [p, q) defines a result
    for (; q < s_len && s[q] != delim; ++q);
    while (q < s_len) {
        s[q] = '\0';
        func(s.data() + p);
        p = q+1;
        for (; q < s_len && s[q] != delim; ++q);
    }
    // process the left chars
    if (p < s_len) {
        func(s.data() + p);
    }
}
//----------------------------------------------------------------------------------------------
class VTChar {
public:
    VTChar()
        : VTChar(0)
    {
    }
    VTChar(const uint32_t& c)
        : mChar(c)
    {
    }
    // operators
    operator uint32_t() const
    {
        return mChar;
    }
    //
    VTChar& operator=(const uint32_t& c)
    {
        mChar = c;
        return *this;
    }
    //
    void SetAttrFuns(const std::vector<std::function<void()> >& attr_funs)
    {
        mAttrFuns = attr_funs;
    }
    //
    void ApplyAttrs() const
    {
        for (size_t k = 0; k < mAttrFuns.size(); ++k) {
            mAttrFuns[k]();
        }
    }
private:
    uint32_t mChar; // UTF-32 LE
    std::vector<std::function<void()> > mAttrFuns;
};

class SerialConnVT : public SerialConn {
public:
    using Superclass = SerialConn;
    SerialConnVT(std::shared_ptr<SerialIo> serial);
    virtual ~SerialConnVT();
    // clear screen and buffer, restore default, .etc, you can override it
    virtual void Clear();
    //
protected:
    // Font
    virtual void Paint(Upp::Draw& draw);
    virtual void Layout();
    virtual bool Key(Upp::dword key, int count);
    virtual void MouseWheel(Upp::Point, int zdelta, Upp::dword);
    virtual void LeftUp(Upp::Point p, Upp::dword keyflags);
    virtual void LeftDown(Upp::Point p, Upp::dword keyflags);
    virtual void MouseMove(Upp::Point p, Upp::dword keyflags);
    virtual void RightUp(Upp::Point p, Upp::dword keyflags);
    virtual Upp::Image CursorImage(Upp::Point p, Upp::dword keyflags);
    //
    typedef std::vector<VTChar> VTLine;
    VTChar mBlankChr;
    //
    std::vector<VTLine> mLinesBuffer; //<! All rendered text
    std::vector<VTLine> mLines; //<! Text on current screen, treat is as virtual screen
    /// Position of virtual cursor
    int mCursorX, mCursorY;
    /// \brief Render text on virtual screen
    /// \param seq complete VT characters
    virtual void RenderText(const std::vector<uint32_t>& s);
    // push to lines buffer and check
    // NOTE: If you want push one line to lines buffer, please use this routine.
    //       DO NOT use push_back directly.
    void PushToLinesBufferAndCheck(const VTLine& vline);
    // check the new line, and push it to the lines buffer if needed.
    virtual void ProcessOverflowLines();
    // Get virtual screen from position p. This routine make a virtual screen from
    // LinesBuffer+Lines.
    // nlines_from_buffer, store the count of lines from lines buffer
    virtual std::vector<VTLine> GetMergedScreen(size_t p, int& nlines_from_buffer) const;
    // calcualte blank lines from end of lines
    int CalculateNumberOfBlankLinesFromEnd(const std::vector<VTLine>& lines) const;
    int CalculateNumberOfBlankCharsFromEnd(const VTLine& vline) const;
    //
    bool mScrollToEnd;
    // selection
    struct SelectionSpan {
        int X0, Y0; // virtual screen, unit: char [fixed width]
        int X1, Y1; // virtual screen, unit: char [fixed height]
        int x0, x1; // logical unit
        int y0, y1; // logical unit
        SelectionSpan()
            : x0(0)
            , y0(0)
            , x1(0)
            , y1(0)
        {
        }
    };
    bool mPressed;
    SelectionSpan mSelectionSpan;
    std::vector<std::string> GetSelection() const;
    Upp::String GetSelectedText() const;
    // return x position of virtual screen
    // lx - unit: logic unit
    int LogicToVirtual(const VTLine& vline, int lx) const;
    // return x position of logic screen
    // vx - unit: char
    int VirtualToLogic(const VTLine& vline, int vx) const;
    // return logic width of vline, equal to VirtualToLogic(vline, 0);
    int GetLogicWidth(const VTLine& vline) const;
    // return longest line size, unit: logic or char
    int GetLongestLineSize(const std::vector<VTLine>& vlines, bool in_logic = true) const;
    // adjust virtual screen, what can affect this routine were listed below
    //  1. Font
    //  2. Size of client region
    // after this function, the ScrollBar and display region were changed.
    virtual void DoLayout();
    // font of console
    Upp::Font mFont;
    int mFontW, mFontH;
    Upp::Color mFgColor; // font color
    Upp::Color mBgColor; // font background color
    Upp::Color mPaperColor; // paper color, \033[0m
    Upp::Color mDefaultFgColor;
    Upp::Color mDefaultBgColor;
    //
    volatile bool mBlinkSignal; // 0,1,0,1,0,1, 2 Hz
    volatile bool mBlink;
    volatile bool mVisible;
    const int kBlinkTimerId = 0;
    // before render character, use this attribute function
    std::vector<std::function<void()> > mCurrAttrFuncs;
    // scroll bar
    Upp::VScrollBar mSbV;
    Upp::HScrollBar mSbH;
    //
    int GetCharWidth(const VTChar& c) const;
    // calculate the size of console
    virtual Upp::Size GetConsoleSize() const;
    // VTChar contains the codepoint
    virtual std::string TranscodeToUTF8(const VTChar& cc) const;
    //
    virtual int IsControlSeq(const std::string& seq) = 0;
    virtual void ProcessControlSeq(const std::string& seq, int seq_type);
    // 00~0x1f
    virtual void ProcessAsciiControlChar(char cc);
    // with K_DELTA
    virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
    virtual bool ProcessKeyUp(Upp::dword key, Upp::dword flags);
    // character
    virtual bool ProcessChar(Upp::dword cc);
    //
    virtual void SetDefaultStyle();
    // render text
    virtual void Render(Upp::Draw& draw);
    // vx    - the position of vchar in vline, we render vchars from this position
    // vy    - the position of vline in virtual screen
    //
    //       |````````````````````````````|
    //       |       lines buffer         |        lines buffer , var: mLinesBuffer
    //       |                            |
    //       |                            |
    //       `````````````````````````````
    //       |       virtual screen       |        virtual screen, var: mLines
    //       |                            |
    //       `````````````````````````````
    // vline - vline to draw
    virtual void DrawVLine(Upp::Draw& draw, int vx, int vy, const VTLine& vline);
    virtual void DrawVLines(Upp::Draw& draw, const std::vector<VTLine>& vlines);
    virtual void DrawCursor(Upp::Draw& draw, int vx, int vy);
    virtual void DrawText(Upp::Draw& draw, int x, int y, const std::string& text,
        const Upp::Font& font, const Upp::Color& cr);
private:
    // receiver
    volatile bool mRxShouldStop;
    void RxProc();
    //
    std::thread mRxThr;
    size_t mMaxLinesBufferSize;
    /// the x,y is absolute position, unit: char, virtual screen.
    inline bool IsCharInSelectionSpan(int x, int y) const;
    //
    void InstallUserActions();
};

#endif
