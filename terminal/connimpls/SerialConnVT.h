//
// (c) 2020 chiv
//
// 2020/9/4 - From now on, CursorX, CursorY is the data position, not presentation position
//            anymore.
//
// This is the base class of virtual terminals
//
#ifndef _comxd_ConnVT_h_
#define _comxd_ConnVT_h_

#include "ColorTable.h"
#include "Conn.h"
#include "VTTypes.h"
#include "VTModes.h"
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
////////////////////////////////////////////////////////////////////////////////////////////////
//                   IMPORTANT INFORMATION
//  1. DO NOT INVOKE ANY FUNCTIONS IN THE HANDLERs OF VT Sequences
//  2. Before accessing any resources of VT, please Acquire the LOCK firstly
//     VT resources includes :
//       a. Lines
//       b. BufferLines
//       c. Style, show cursor, wrap line, .etc
//       d. Vx,Vy, Px,Py
//       e. ColorTable
//       f. ScrollRegion
//       g. Modes
//     In the Handlers, you can access these resources freely without lock them.
//
////////////////////////////////////////////////////////////////////////////////////////////////
namespace Upp {
    class DebugMutex : public NoCopy {
    public:
        class Lock {
        public:
            Lock(DebugMutex& mtx)
                : mMtx(mtx)
            {
                mtx.Enter();
            }
            virtual ~Lock()
            {
                mMtx.Leave();
            }
        private:
            DebugMutex& mMtx;
        };
        
        void Enter()
        {
            mMtx.Enter();
            mOwnerId = std::this_thread::get_id();
        }
        void Leave()
        {
            mMtx.Leave();
        }
        
        std::thread::id GetOwnerId() const { return mOwnerId; }
        
    private:
        Mutex mMtx;
        //
        std::thread::id mOwnerId;
    };
}
#define ENABLE_THREAD_CHECK 0
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#if ENABLE_THREAD_CHECK
#define CHECK_GUI_THREAD() do { \
    if (GetOwnerId() != std::this_thread::get_id()) { \
        std::string report = std::string("You must invoke ") + __FUNCTION__ + " from GUI thread"; \
        PromptOK(DeQtf(report.c_str())); \
        abort(); \
    } \
} while (0)
// This codes maybe run in Non GUI thread, in that case, the Framework will report an error, or
// the program will block forever on acquiring Gui Lock...
#define CHECK_LOCK_VT() do { \
    if (mLockVt.GetOwnerId() != std::this_thread::get_id()) { \
        std::string report = std::string("You must acuire the LockVt before invoking ") + __FUNCTION__; \
        PromptOK(DeQtf(report.c_str())); \
        abort(); \
    } \
} while (0)
#else
#define CHECK_GUI_THREAD()
#define CHECK_LOCK_VT()
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class ControlSeqFactory;
class SerialConnVT : public SerialConn {
public:
    using Superclass = SerialConn;
    SerialConnVT(std::shared_ptr<SerialIo> io);
    virtual ~SerialConnVT();
    // start the works
    virtual bool Start();
    // stop the works
    virtual void Stop();
    // clear screen and buffer, restore default, .etc, you can override it
    virtual void Clear();
    // Editing commands
    virtual void SelectAll();
    //
    struct SelectionSpan {
        int X0, Y0; // virtual screen, unit: char [fixed width]
        int X1, Y1; // virtual screen, unit: char [fixed height]
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
    //
    void SetWrapLine(bool b);
    void SetShowCursor(bool b);
    //
    Upp::Event<Upp::Bar&> WhenBar;

protected:
    ControlSeqFactory* mSeqsFactory;
    std::map<int, std::function<void(const std::string&)>> mFunctions;
    //
    void RunParserBenchmark();
    //
    void ShowVTOptions();
    //
    // public methods
    struct ScreenData {
        std::deque<VTLine> LinesBuffer;
        std::deque<VTLine> Lines; // virtual screen
        VTStyle Style;
        int Vx, Vy;
        struct SelectionSpan SelSpan;
        ScreenData()
            : Vx(0)
            , Vy(0)
        {
        }
    };
    void SaveScr(ScreenData& sd);
    void LoadScr(const ScreenData& sd);
    // swap current scr and sd
    void SwapScr(ScreenData& sd);
    //------------------------------------------------------------------------------------------
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
    // virtual screen resources
    VTChar mBlankChar;
    Upp::Font mFont;
    int mFontW, mFontH;
    VTStyle mStyle; // current style
    VTColorTable mColorTbl;
    bool mBlinkSignal; // 0,1,0,1,0,1, 2 Hz
    std::deque<VTLine> mLinesBuffer;
    std::deque<VTLine> mLines; //<! Text on current screen, treat is as virtual screen
    Upp::Size          mVtSize; // size of virtual terminal, in CELLs
    Upp::Size          mVwSize; // size of View, in Pixels
    Upp::Size GetConsoleSize() const { return mVtSize; }
    Upp::Size GetViewSize() const { return mVwSize; }
    //-------------------------------------------------------------------------------------------
    Upp::DebugMutex mLockVt;   // protect virtual screen, before accessing VtSize, Lines, .etc
                               // you must acquire the LockVt.
    //--------------------------------------------------------------------------------------------
    void ClearVt();
    //
    struct Seq {
        enum SeqType {
            CTRL_SEQ,
            TEXT_SEQ,
            NULL_SEQ
        };
        int Type;
        std::pair<int, std::string> Ctrl;
        std::vector<uint32_t> Text;
        Seq(int seq_type, const char* p)
        {
            Type = CTRL_SEQ;
            Ctrl = std::make_pair(seq_type, p);
        }
        Seq(int seq_type, const char* p, size_t sz)
        {
            Type = CTRL_SEQ;
            Ctrl = std::make_pair(seq_type, std::string(p, p + sz));
        }
        Seq(std::vector<uint32_t>&& seq)
        {
            Type = TEXT_SEQ;
            Text = std::move(seq);
        }
        Seq(const std::vector<uint32_t>& text)
        {
            Type = TEXT_SEQ;
            Text = text;
        }
        Seq()
        {
            Type = NULL_SEQ;
        }
    };
    //
    size_t ParseSeqs(const char* input, size_t input_sz, std::deque<struct Seq>& seqs);
    //
    virtual void RenderSeq(const Seq& seq);
    virtual void RenderSeqs(const std::deque<Seq>& seqs);
    //
    virtual void Put(const std::string& s);
    //-------------------------------------------------------------------------------------
    /// Active position
    int mVx, mVy; //<! Active position of data component
    int mPx, mPy; //<! Active position of presentation component
    /// charset, pair [charset-index, charset-type]
    std::map<int, int> mCharsets; // supported charsets
    int mCharset;
    /// \brief Before rendering text, the program will remap the character,
    ///        the derived class could modify the character to override this method
    virtual uint32_t RemapCharacter(uint32_t uc, int charset);
    /// \brief render text to VTChar
    /// \param seq complete VT characters
    virtual void RenderText(const std::vector<uint32_t>& s);
    // push to lines buffer and check
    // NOTE: If you want push one line to lines buffer, please use this routine.
    //       DO NOT use push_back directly.
    void PushToLinesBufferAndCheck(const VTLine& vline);
    bool ProcessOverflowLines();
    bool ProcessOverflowChars();
    virtual bool ProcessOverflowLines(const struct Seq& seq);
    virtual bool ProcessOverflowChars(const struct Seq& seq);
    // calcualte blank lines from end of lines
    int CalculateNumberOfBlankLinesFromEnd(const std::deque<VTLine>& lines) const;
    int CalculateNumberOfBlankCharsFromEnd(const VTLine& vline) const;
    int CalculateNumberOfPureBlankCharsFromEnd(const VTLine& vline) const;
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
    Upp::Point LogicToVirtual(int lx, int ly, int& px, int& next_px, int& py, int& next_py, bool ignore_tail_blanks = true);
    Upp::Point VirtualToLogic(int vx, int vy, bool ignore_tail_blanks = true);
    // lx, ly - lines is virtual screen
    //        cN       cN+1   cN+2
    // lx     |____lx___|______|
    Upp::Point LogicToVirtual(const std::deque<VTLine>& lines, int lx, int ly, int& px, int& next_px,
        int& py, int& next_py, bool ignore_tail_blanks = true);
    Upp::Point VirtualToLogic(const std::deque<VTLine>& lines, int vx, int vy, bool ignore_tail_blanks = true);
    //        cN       cN+1   cN+2
    // lx     |____lx___|______|
    int LogicToVirtual(const VTLine& vline, int lx, int& px, int& next_px, bool ignore_tail_blanks = true);
    int VirtualToLogic(const VTLine& vline, int vx, bool ignore_tail_blanks = true);
    // return logic width of vline, unit: pixels
    int GetLogicWidth(const VTLine& vline, int count = -1, bool ignore_tail_blanks = true);
    // adjust virtual screen, what can affect this routine were listed below
    //  1. Font
    //  2. Size of client region
    // after this function, the ScrollBar and display region were changed.
    virtual void ResizeVt(const Upp::Size& csz);
    //
    void UseStyle(const VTChar& c, Upp::Font& font, Upp::Color& fg_color, Upp::Color& bg_color,
        bool& blink, bool& visible);
    //
    Upp::TimeCallback mBlinkTimer;
    // scroll bar
    Upp::VScrollBar mSbV;
    Upp::HScrollBar mSbH;
    //
    virtual int GetCharWidth(const VTChar& c) const;
    // vy - absolute position
    virtual int GetLineHeight(int vy) const;
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
    // before finding control sequences, we could refine the input.
    virtual void RefineTheInput(std::string& raw);
    virtual bool IsControlSeqPrefix(uint8_t c);
    /// When ?
    /// The receiver will analyze the buffer received, when it come across a character
    /// unrecognized, the IsControlSeq was invoked to defined the seq. The return value
    /// of IsControlSeq defines the next step.
    /// - SEQ_NONE, the seq was discarded
    /// - SEQ_PENDING, need more chars to define farther, the receiver will append one char
    ///                and invoke IsControlSeq again
    /// - Valid Seq Type, the receiver will add the seq and it's type to the render-queue,
    ///   it will be processed later.
    ///   If IsControlSeq returns true, it should set the p_begin, p_sz to tell the receiver
    ///   the location and size of the parameter.
    virtual int IsControlSeq(const char* input, size_t input_sz, size_t& p_begin, size_t& p_sz, size_t& s_end);
    /// process the seq
    /// @param seq_type Type of sequence
    /// @param p Parameter of this sequence
    virtual bool ProcessControlSeq(int seq_type, const std::string& p);
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
    int GetVTLinesHeight(const std::deque<VTLine>& lines) const;
    //
    virtual void UpdatePresentationPos(int flags = -1); // Vx,Vy -> Px,Py
    virtual void UpdateDataPos(int flags = -1); // Px,Py -> Vx,Vy
    virtual void UpdateHScrollbar();
    virtual void UpdateVScrollbar();
    virtual void UpdatePresentation();
    // vy - global position of Y
    VTLine* GetVTLine(int vy);
    const VTLine* GetVTLine(int vy) const;
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
    void CheckAndFix(ScrollingRegion& span);
    // all modes defined here
    VTModes mModes;
    //------------------------------------------------------------------------------------------
    int mTabWidth;
    //------------------------------------------------------------------------------------------
    bool IsCharInSelectionSpan(int vx, int vy, const SelectionSpan& selection_span) const;
    //
    std::thread::id GetOwnerId() const { return mOwnerId; }
private:
    bool mWrapLine;
    bool mScrollToEnd;
    bool mPressed;
    bool mShowCursor;
    int mBackgroundColorId;
    int mForegroundColorId;
    //
    std::thread::id mOwnerId;
    // receiver
    volatile bool mRxShouldStop;
    void RxProc();
    //
    void ExtendVirtualScreen(int cx, int cy);
    void ShrinkVirtualScreen(int cx, int cy);
    //
    std::thread mRxThr;
    size_t mMaxLinesBufferSize;
    // vx,vy Position of data
    inline bool IsCharInSelectionSpan(int vx, int vy) const;
    //
    void InstallUserActions();
};

#endif
