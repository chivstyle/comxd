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
    SerialConnVT(std::shared_ptr<serial::Serial> serial);
    virtual ~SerialConnVT();
    //
    std::list<const UsrAction*> GetActions() const;
    // clear screen and buffer
    void Clear();
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
    virtual void MouseLeave();
    virtual Upp::Image CursorImage(Upp::Point p, Upp::dword keyflags);
    //
    void ShowOptionsDialog();
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
    // push to lines buffer and check, fix if needed
    void PushToLinesBufferAndCheck(const VTLine& vline);
    void ProcessOverflowLines();
    std::vector<VTLine> GetBufferLines(size_t p, int& y);
    // calcualte blank lines from end of lines
    int CalculateNumberOfBlankLinesFromEnd(const std::vector<VTLine>& lines) const;
    int CalculateNumberOfBlankCharsFromEnd(const VTLine& vline) const;
    //
    bool mScrollToEnd;
    // selection
    struct SelectionSpan {
        int X0, Y0;
        int X1, Y1;
        SelectionSpan()
            : X0(0)
            , Y0(0)
            , X1(0)
            , Y1(0)
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
    //
    UsrAction mActOptions; // Action to show options dialog
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
    Upp::Size GetConsoleSize() const;
    //
    virtual int IsControlSeq(const std::string& seq) = 0;
    virtual void ProcessControlSeq(const std::string& seq, int seq_type) = 0;
    virtual void ProcessAsciiControlChar(unsigned char cc);
    // with K_DELTA
    virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
    virtual bool ProcessKeyUp(Upp::dword key, Upp::dword flags);
    // pure ascii, 32 ~ 126
    virtual bool ProcessKeyDown_Ascii(Upp::dword key, Upp::dword flags);
    //
    virtual void SetDefaultStyle();
    // render text
    virtual void Render(Upp::Draw& draw);
    virtual void Render(Upp::Draw& draw, const std::vector<VTLine>& vlines);
    // handlers to process trivial seqs
    typedef std::function<void()> ControlHandler;
    std::map<std::string, ControlHandler> mCtrlHandlers;
private:
    // receiver
    volatile bool mRxShouldStop;
    void RxProc();
    //
    std::thread mRxThr;
    size_t mMaxLinesBufferSize;
    /// the x,y is absolute position, unit: char
    inline bool IsCharInSelectionSpan(int x, int y) const;
};

#endif
