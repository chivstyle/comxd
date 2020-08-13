//
// (c) 2020 chiv
//
#ifndef _comxd_ConnVT102_h_
#define _comxd_ConnVT102_h_

#include "SerialConnVT.h"

class SerialConnVT102 : public SerialConnVT {
public:
    using Superclass = SerialConnVT;
    SerialConnVT102(std::shared_ptr<serial::Serial> serial);
    virtual ~SerialConnVT102();
    // public methods
    //-------------------------------------------------------------------------------------
    struct CursorData {
        // position
        int X, Y;
        // attributes
        bool Bold, Italic, Strikeout, Underline;
        // color
        Upp::Color FgColor, BgColor;
        CursorData()
            : X(0)
            , Y(0)
            , Bold(false)
            , Italic(false)
            , Strikeout(false)
            , Underline(false)
        {
        }
    };
    void SaveCursor(CursorData& cd); // ESC 7
    void LoadCursor(const CursorData& cd); // ESC 8
protected:
    /// workflow
    ///
    /// rx_buffer, check byte one by one, then
    ///    seq_type = IsControlSeq(seq)
    ///    seq_type == 0, can't recognize the control seq, treat it as normal text
    ///    seq_type == 1, pending, need more bytes to confirm
    ///    seq_type == 2, trivial control seq, it's in the table 'kVT102ControlSeqs'
    ///    seq_type > 2, other kinds of control seq
    /// then, RenderText(seq, seq_type)
    ///    ProcessAsciiControlChar(cc), cc is 8, 9, 0xd, 0xa, .etc
    ///    ProcessControlSeq(seq), seq matches control seq pattern
    ///
    virtual int IsControlSeq(const std::string& seq);
    //-------------------------------------------------------------------------------------
    // you can override these two routines to modify all functions.
    virtual void ProcessControlSeq(const std::string& seq, int seq_type);
    virtual void ProcessAsciiControlChar(char cc);
    //-------------------------------------------------------------------------------------
    // with K_DELTA
    virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
    virtual bool ProcessKeyUp(Upp::dword key, Upp::dword flags);
    // pure ascii, 32 ~ 126
    virtual bool ProcessKeyDown_Ascii(Upp::dword key, Upp::dword flags);
    //-------------------------------------------------------------------------------------
    // allow the subclass to extend the functions of VT102.
    virtual void ProcessVT102Trivial(const std::string& seq);
    // Process Cursor Movement Commands with variable parameters.
    virtual void ProcessVT102CursorMovementCmds(const std::string& seq);
    virtual void ProcessVT102EditingFunctions(const std::string& seq);
    virtual void ProcessVT102CharAttributes(const std::string& seq);
    virtual void ProcessVT102ScrollingRegion(const std::string& seq);
    // VT102 subroutines
    virtual void ProcessVT102CharAttribute(int attr_code);
    //-------------------------------------------------------------------------------------
    virtual Upp::Size GetConsoleSize() const;
    virtual void ProcessOverflowLines();
    virtual std::vector<VTLine> GetMergedScreen(size_t p, int& nlines_from_buffer) const;
    virtual void DrawCursor(Upp::Draw& draw, int vx, int vy);
    virtual void DoLayout();
    void ExtendVirtualScreen(int cx, int cy);
    void ShrinkVirtualScreen(int cx, int cy);
    //-------------------------------------------------------------------------------------
private:
    std::map<std::string, std::function<void()> > mVT102TrivialHandlers;
    void InstallVT102Functions();
    //
    CursorData mCursorData;
    //-------------------------------------------------------------------------------------
    // We do not use all of them. for example, the VT has unlimited column size, so
    // C132/C80,AutoWrap are not needed.
    struct VT102Modes {
        enum BooleanValue {
            Off, On
        };
        enum KAM {
            Locked, Unlocked
        };
        int KeyboardAction: 1;
        enum IRM {
            Insert, Replace
        };
        int InsertionReplacement: 1;
        int SendReceive: 1;
        enum LMN {
            NewLine, LineFeed
        };
        int LineFeedNewLine: 1;
        enum DECCKM {
            Application, Cursor
        };
        int CursorKey: 1;
        enum DECANM {
            ANSI, VT52
        };
        int AnsiVT52: 1;
        enum DECCOLM {
            C132, C80
        };
        int Column: 1;
        enum DECSCLM {
            Smooth, Jump
        };
        int Scrolling: 1;
        enum DECSCNM {
            Reverse, Normal
        };
        int Screen: 1;
        enum DECOM {
            Relative, Absolute
        };
        int Origin: 1;
        int AutoWrap: 1;
        int AutoRepeat: 1;
        int PrintFormFeed: 1;
        enum DECPES {
            // FullScreen is LinesBuffer+Lines
            // ScrollingRegion is Lines
            FullScreen, ScrollingRegion
        };
        int PrintExtent: 1;
        //
        VT102Modes()
            : KeyboardAction(Unlocked)
            , InsertionReplacement(Replace)
            , SendReceive(On)
            , LineFeedNewLine(NewLine)
            , CursorKey(Cursor)
            , AnsiVT52(ANSI)
            , Column(C132)
            , Scrolling(Smooth)
            , Screen(Normal)
            , Origin(Absolute)
            , AutoWrap(Off)
            , AutoRepeat(Off)
            , PrintFormFeed(Off)
            , PrintExtent(FullScreen)
        {
        }
    };
    VT102Modes mModes;
    struct ScrollingRegion {
        int Top, Bottom;
        ScrollingRegion()
            : Top(0)
            , Bottom(-1)
        {
        }
    };
    ScrollingRegion mScrollingRegion;
};

#endif
