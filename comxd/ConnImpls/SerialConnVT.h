//
// (c) 2020 chiv
//
#ifndef _comxd_ConnVT_h_
#define _comxd_ConnVT_h_

#include "Conn.h"
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <thread>

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
    class VTChar {
    public:
        VTChar()
            : VTChar('\0')
        {
        }
        VTChar(const char c)
            : mC(c)
        {
        }
        // operators
        operator char() const
        {
            return mC;
        }
        VTChar& operator=(const char& c)
        {
            mC = c;
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
        char mC;
        std::vector<std::function<void()> > mAttrFuns;
    };
    typedef std::vector<VTChar> VTLine;
    VTChar mBlankChr;
    //
    std::mutex mLinesLock; //<! Protect virtual screen
    std::vector<VTLine> mLinesBuffer; //<! All rendered text
    std::vector<VTLine> mLines; //<! Text on current screen, treat is as virtual screen
    /// Position of virtual cursor
    int mCursorX, mCursorY;
    /// \brief Render text on virtual screen
    /// \param seq complete VT characters
    virtual void RenderText(const std::string& seq, int seq_type);
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
    /// the x,y is relative position.
    inline bool IsCharInSelectionSpan(int x, int y) const;
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
    //
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
    // handlers to process trivial control seqs
    void InstallControlSeqHandlers();
    // receiver
    volatile bool mRxShouldStop;
    void RxProc();
    //
    std::thread mRxThr;
    size_t mMaxLinesBufferSize;
};

#endif
