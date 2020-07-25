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
    virtual void MouseLeave();
    virtual Upp::Image& CursorOverride();
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
            , mAttrFun([]() {})
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
        void SetAttrFun(std::function<void()> attr_fun)
        {
            mAttrFun = attr_fun;
        }
        void ApplyAttr() const { mAttrFun(); }
    private:
        char mC;
        std::function<void()> mAttrFun;
    };
    typedef std::vector<VTChar> VTLine;
    VTChar mBlankChr;
    //
    std::mutex mLinesLock; //<! Protect virtual screen
    std::vector<VTLine> mLinesBuffer; //<! All rendered text
    std::vector<VTLine> mLines; //<! Text on current screen, treat is as virtual screen
    int mRealLines; //<! real lines in the virtual screen
    int mRealX;
    /// Position of virtual cursor
    int mCursorX, mCursorY;
    /// \brief Render text on virtual screen
    /// \param seq complete VT characters
    void RenderText(const std::string& seq);
    //
    std::vector<VTLine> GetBufferLines(size_t p, int& y);
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
    Upp::Image mImageIbeam;
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
    const int kBlinkTimerId = 0;
    // before render character, use this attribute function
    std::function<void()> mCurrAttrFunc;
    // scroll bar
    Upp::VScrollBar mSbV;
    //
    Upp::Size GetConsoleSize() const;
    //
    virtual int IsControlSeq(const std::string& seq);
    // This is VT102 cursor key codes
    virtual void ProcessVT102CursorKeyCodes(const std::string& seq);
    virtual void ProcessAsciiControlChar(unsigned char cc);
    virtual void InstallVT102ControlSeqHandlers();
    virtual void ProcessControlSeq(const std::string& seq);
    virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
    virtual bool ProcessKeyUp(Upp::dword key, Upp::dword flags);
    //
    virtual void SetDefaultStyle();
    // the position of current pen
    int mX;
    int mY;
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
