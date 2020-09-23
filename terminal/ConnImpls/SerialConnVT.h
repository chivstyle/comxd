//
// (c) 2020 chiv
//
// 2020/9/4 - From now on, CursorX, CursorY is the data position, not presentation position
//            anymore.
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
// UTF-8 to UTF-32, use ? for invalid UTF-8 seq.
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
            // check and check
            if ((seq[p+1] & 0xc0) != 0x80 ||
                (seq[p+2] & 0xc0) != 0x80 ||
                (seq[p+3] & 0xc0) != 0x80)
            {
                out.push_back('?');
                p++;
            } else {
                bits |= (seq[p+1] & 0x3f) << 12;
                bits |= (seq[p+2] & 0x3f) << 6;
                bits |= (seq[p+3] & 0x3f);
                out.push_back(bits);
                p += 4;
            }
        } else if ((flag & 0xe0) == 0xe0) { // 3 bytes, 4+6+6=16bits
            if (seq.size() - p < 3) break;
            if ((seq[p+1] & 0xc0) != 0x80 ||
                (seq[p+2] & 0xc0) != 0x80)
            {
                out.push_back('?');
                p++;
            } else {
                uint32_t bits = (seq[p] & 0x0f) << 12;
                bits |= (seq[p+1] & 0x3f) << 6;
                bits |= (seq[p+2] & 0x3f);
                out.push_back(bits);
                p += 3;
            }
        } else if ((flag & 0xc0) == 0xc0) { // 2 bytes, 5+6 = 11bits
            if (seq.size() - p < 2) break;
            if ((seq[p+1] & 0xc0) != 0x80) {
                out.push_back('?');
                p++;
            } else {
                uint32_t bits = (seq[p] & 0x1f) << 6;
                bits |= (seq[p+1] & 0x3f);
                out.push_back(bits);
                p += 2;
            }
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
        : VTChar(c, std::vector<std::function<void()> >())
    {
    }
    VTChar(const uint32_t& c, const std::vector<std::function<void()> >& attr_funs)
        : mChar(c)
        , mCx(0)
        , mCy(0)
        , mAttrFuns(attr_funs)
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
    // cx, cy
    void SetCx(int cx) { mCx = cx; }
    void SetCy(int cy) { mCy = cy; }
    int Cx() const { return mCx; }
    int Cy() const { return mCy; }
    //
private:
    uint32_t mChar; // UTF-32 LE
    int mCx;
    int mCy;
    std::vector<std::function<void()> > mAttrFuns;
};
/// warning: You should set height manually.
class VTLine : public std::vector<VTChar> {
public:
    VTLine() {}
    explicit VTLine(size_t sz)
        : std::vector<VTChar>(sz)
        , mHeight(1)
    {
    }
    VTLine(size_t sz, const VTChar& c)
        : std::vector<VTChar>(sz, c)
        , mHeight(1)
    {
    }
    VTLine& SetHeight(int height) { mHeight = height; return *this; }
    int GetHeight() const { return mHeight; };
    
private:
    int mHeight;
};

class SerialConnVT : public SerialConn {
public:
    using Superclass = SerialConn;
    SerialConnVT(std::shared_ptr<SerialIo> serial);
    virtual ~SerialConnVT();
    // clear screen and buffer, restore default, .etc, you can override it
    virtual void Clear();
    // Editing commands
    virtual void SelectAll();
    //
    struct SelectionSpan {
        int X0, Y0; // virtual screen, unit: char [fixed width]
        int X1, Y1; // virtual screen, unit: char [fixed height]
        int x0, y0; // reserved for define the whether the span is valid.
        bool Valid;
        SelectionSpan()
            : X0(0)
            , Y0(0)
            , X1(0)
            , Y1(0)
            , Valid(false)
        {
        }
    };
    // public methods
    struct ScreenData {
        std::vector<VTLine> LinesBuffer; // buffer
        std::vector<VTLine> Lines;       // virtual screen
        std::vector<std::function<void()> > AttrFuncs;
        int Vx, Vy;
        Upp::Font Font;
        Upp::Color FgColor, BgColor;
        bool Blink;
        struct SelectionSpan SelSpan;
    };
    void SaveScr(ScreenData& sd);
    void LoadScr(const ScreenData& sd);
    //
    struct CursorData {
        int Vx, Vy;
        bool Blink;
        bool Strikeout, Bold, Italic, Underline;
        Upp::Color BgColor, FgColor;
        std::vector<std::function<void()> > AttrFuncs;
    };
    void SaveCursor(CursorData& cd);
    void LoadCursor(const CursorData& cd);
protected:
    // Font
    virtual void Paint(Upp::Draw& draw);
    virtual void Layout();
    virtual bool Key(Upp::dword key, int count);
    virtual void MouseWheel(Upp::Point, int zdelta, Upp::dword);
    virtual void LeftUp(Upp::Point p, Upp::dword keyflags);
    virtual void LeftDown(Upp::Point p, Upp::dword keyflags);
    virtual void LeftDouble(Upp::Point p, Upp::dword keyflags);
    virtual void LeftTriple(Upp::Point p, Upp::dword keyflags);
    virtual void MouseMove(Upp::Point p, Upp::dword keyflags);
    virtual void RightUp(Upp::Point p, Upp::dword keyflags);
    virtual Upp::Image CursorImage(Upp::Point p, Upp::dword keyflags);
    //
    VTChar mBlankChar;
    // If the lines buffer is very very long, we should modify LogicToVirtual, VirtualToLogic
    // to improve performance. For example, let's add position to VTLine, then you can use
    // binary_search to find the position, i.e the logic position and virtual position pair.
    // Now, the size of lines buffer is limited to 20000, it's a small number for modern computer, so
    // we do nothing to keep the code simple and stupid.
    std::vector<VTLine> mLinesBuffer; //<! All rendered text
    std::vector<VTLine> mLines; //<! Text on current screen, treat is as virtual screen
    /// Active position
    int mVx, mVy;      //<! Active position of data component
    int mPx, mPy;      //<! Active position of presentation component
    /// \brief Render text on virtual screen
    /// \param seq complete VT characters
    virtual void RenderText(const std::vector<uint32_t>& s);
    // push to lines buffer and check
    // NOTE: If you want push one line to lines buffer, please use this routine.
    //       DO NOT use push_back directly.
    void PushToLinesBufferAndCheck(const VTLine& vline);
    // 1. If mVy is out of range, push the overflow lines to buffer
    // 2. If mVx is out of range, extend the line in position mVy.
    virtual void ProcessOverflowLines();
    // calcualte blank lines from end of lines
    int CalculateNumberOfBlankLinesFromEnd(const std::vector<VTLine>& lines) const;
    int CalculateNumberOfBlankCharsFromEnd(const VTLine& vline) const;
    //
    bool mScrollToEnd;
    bool mPressed;
    //
    SelectionSpan mSelectionSpan;
    std::vector<std::string> GetSelection() const;
    Upp::String GetSelectedText() const;
    //---------------------------------------------------------------------------------------
    virtual void Copy();
    virtual void Paste();
    //---------------------------------------------------------------------------------------
    // lx, ly - Absolute position
    //        cN       cN+1   cN+2
    // lx     |____lx___|______|
    Upp::Point LogicToVirtual(int lx, int ly, int& px, int& next_px, int& py, int& next_py);
    Upp::Point VirtualToLogic(int vx, int vy);
    // lx, ly - lines is virtual screen
    //        cN       cN+1   cN+2
    // lx     |____lx___|______|
    Upp::Point LogicToVirtual(const std::vector<VTLine>& lines, int lx, int ly, int& px, int& next_px,
                                                                                int& py, int& next_py);
    Upp::Point VirtualToLogic(const std::vector<VTLine>& lines, int vx, int vy);
    //        cN       cN+1   cN+2
    // lx     |____lx___|______|
    int LogicToVirtual(const VTLine& vline, int lx, int& px, int& next_px);
    int VirtualToLogic(const VTLine& vline, int vx);
    // return logic width of vline, unit: pixels
    int GetLogicWidth(const VTLine& vline, int count = -1);
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
    Upp::Color mTextsColor; // default texts color
    //
    volatile bool mBlinkSignal; // 0,1,0,1,0,1, 2 Hz
    volatile bool mBlink;
    volatile bool mVisible;
    Upp::TimeCallback mBlinkTimer;
    // before render character, use this attribute function
    std::vector<std::function<void()> > mCurrAttrFuncs;
    // scroll bar
    Upp::VScrollBar mSbV;
    Upp::HScrollBar mSbH;
    bool mTrackCaret;
    //
    virtual int GetCharWidth(const VTChar& c);
    // vy - absolute position
    virtual int GetLineHeight(int vy) const;
    // calculate the size of console
    virtual Upp::Size GetConsoleSize() const;
    // VTChar contains the codepoint, UTF32->UTF8
    // used by Drawing functions
    virtual Upp::WString TranscodeToUTF16(const VTChar& cc) const;
    /// raw(default:UTF8, we'll support more later)->UTF32
    /// used by Rendering functions, NOTE: Not drawing
    /// --------------------Working flow---------------------------------
    ///   auto recv_buf = receive_from_port()
    ///   control_seq, rest = split(recv_buf)
    ///   ProcessControlSeq(control_seq)
    ///
    ///   size_t ep;
    ///   auto utf32_text = TranscodeToUTF32(rest, ep)
    ///   RenderText(utf32_text)
    ///------------------------------------------------------------------
    /// \param s
    /// \param ep End position of raw string. From ep To end of s, the data could not be
    ///           recognized.
    virtual std::vector<uint32_t> TranscodeToUTF32(const std::string& s, size_t& ep);
    //
    virtual bool IsSeqPrefix(unsigned char c);
    //
    virtual int IsControlSeq(const std::string& seq);
    virtual bool ProcessControlSeq(const std::string& seq, int seq_type);
    // 00~0x1f
    // VT will process following chars
    // 0x0b VT
    // 0x09 HT
    // 0x08 BS
    // 0x0a LN
    // 0x0d CR
    // others were ignored.
    virtual bool ProcessAsciiControlChar(char cc);
    // with K_DELTA
    virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
    virtual bool ProcessKeyUp(Upp::dword key, Upp::dword flags);
    // character
    virtual bool ProcessChar(Upp::dword cc);
    //
    virtual void SetDefaultStyle();
    // render text
    virtual void Render(Upp::Draw& draw);
    // draw cursor
    virtual void DrawCursor(Upp::Draw& draw);
    // draw character
    virtual void DrawVTChar(Upp::Draw& draw, int x, int y, const VTChar& c,
                            const Upp::Font& font, const Upp::Color& cr);
    //
    virtual void DrawVT(Upp::Draw& draw);
    virtual void DrawVTLine(Upp::Draw& draw, const VTLine& vline,
                            int vx, int vy, /*! absolute position of data */
                            int lxoff, int lyoff);
    //
    int GetVTLinesHeight(const std::vector<VTLine>& lines) const;
    //
    virtual void UpdatePresentationPos();
    virtual void UpdateHScrollbar();
    virtual void UpdateVScrollbar();
    virtual void UpdatePresentation();
    //
    VTLine* GetVTLine(int vy);
    //------------------------------------------------------------------------------------------
    // Scrolling region defines a subsequent page
    struct ScrollingRegion {
        int Top, Bottom;
        ScrollingRegion()
            : Top(0)
            , Bottom(-1)
        {
        }
    };
    ScrollingRegion mScrollingRegion;
    //------------------------------------------------------------------------------------------
private:
    void ExtendVirtualScreen(int cx, int cy);
    void ShrinkVirtualScreen(int cx, int cy);
    // receiver
    volatile bool mRxShouldStop;
    void RxProc();
    //
    std::thread mRxThr;
    size_t mMaxLinesBufferSize;
    // vx,vy Position of data
    inline bool IsCharInSelectionSpan(int vx, int vy) const;
    //
    void InstallUserActions();
};

#endif
