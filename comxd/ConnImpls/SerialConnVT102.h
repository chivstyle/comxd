//
// (c) 2020 chiv
//
#ifndef _comxd_ConnVT102_h_
#define _comxd_ConnVT102_h_

#include "Conn.h"
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <thread>

class SerialConnVT102 : public SerialConn {
public:
    SerialConnVT102(std::shared_ptr<serial::Serial> serial);
    virtual ~SerialConnVT102();
    //
    virtual std::list<const UsrAction*> GetActions() const;
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
    void RenderText(const std::string& seq, int seq_type);
    // push to lines buffer and check, fix if needed
    inline void PushToLinesBufferAndCheck(const VTLine& vline);
    inline void ProcessOverflowLines();
    std::vector<VTLine> GetBufferLines(size_t p, int& y);
    // calcualte blank lines from end of lines
    inline int CalculateNumberOfBlankLinesFromEnd(const std::vector<VTLine>& lines) const;
    inline int CalculateNumberOfBlankCharsFromEnd(const VTLine& vline) const;
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
    virtual int IsControlSeq(const std::string& seq);
    // This is VT102 cursor key codes
    virtual void ProcessVT102CursorKeyCodes(const std::string& seq);
    virtual void ProcessVT102EditingFunctions(const std::string& seq);
    virtual void ProcessVT102Attrs(const std::string& seq);
    virtual void ProcessAttr(const std::string& attr_code);
    virtual void ProcessControlSeq(const std::string& seq, int seq_type);
    virtual void ProcessAsciiControlChar(unsigned char cc);
    virtual void InstallVT102ControlSeqHandlers();
    // with K_DELTA
    virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
    virtual bool ProcessKeyUp(Upp::dword key, Upp::dword flags);
    // pure ascii, 32 ~ 126
    virtual bool ProcessKeyDown_Ascii(Upp::dword key, Upp::dword flags);
    //
    virtual void SetDefaultStyle();
    // the position of current pen
    int mX;
    int mY;
    size_t mMaxLinesBufferSize;
    // render text
    void Render(Upp::Draw& draw);
    void Render(Upp::Draw& draw, const std::vector<VTLine>& vlines);
    //
    typedef std::function<void()> ControlHandler;
    std::map<std::string, ControlHandler> mCtrlHandlers;
    // receiver
    volatile bool mRxShouldStop;
    void RxProc();
    //
    std::thread mRxThr;
};

#endif
