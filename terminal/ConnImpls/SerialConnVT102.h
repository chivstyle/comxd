//
// (c) 2020 chiv
//
#ifndef _comxd_ConnVT102_h_
#define _comxd_ConnVT102_h_

#include "SerialConnVT.h"

class SerialConnVT102 : public virtual SerialConnVT {
public:
    using Superclass = SerialConnVT;
    SerialConnVT102(std::shared_ptr<SerialIo> serial);
    virtual ~SerialConnVT102();
protected:
    //
    virtual int IsControlSeq(const std::string& seq);
    //-------------------------------------------------------------------------------------
    // you can override these two routines to modify all functions.
    virtual bool ProcessControlSeq(const std::string& seq, int seq_type);
    // C0 set.
    virtual bool ProcessAsciiControlChar(char cc);
    //-------------------------------------------------------------------------------------
    //
    // key, must be
    // 1. key with K_DELTA
    // 2. RETURN
    virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
    virtual bool ProcessKeyUp(Upp::dword key, Upp::dword flags);
    //
    virtual bool ProcessChar(Upp::dword cc);
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
    std::string TranscodeToUTF8(const VTChar& cc) const;
private:
    //
    std::map<std::string, std::function<void()> > mVT102TrivialHandlers;
    void InstallVT102Functions();
    // ESC#7,#8 need this data.
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
        unsigned int KeyboardAction: 1;
        enum IRM {
            Insert, Replace
        };
        unsigned int InsertionReplacement: 1;
        unsigned int SendReceive: 1;
        enum LMN {
            NewLine, LineFeed
        };
        unsigned int LineFeedNewLine: 1;
        enum DECCKM {
            Application, Cursor
        };
        unsigned int CursorKey: 1;
        enum DECANM {
            ANSI, VT52
        };
        unsigned int AnsiVT52: 1;
        enum DECCOLM {
            C132, C80
        };
        unsigned int Column: 1;
        enum DECSCLM {
            Smooth, Jump
        };
        unsigned int Scrolling: 1;
        enum DECSCNM {
            Reverse, Normal
        };
        unsigned int Screen: 1;
        enum DECOM {
            Relative, Absolute
        };
        unsigned int Origin: 1;
        unsigned int AutoWrap: 1;
        unsigned int AutoRepeat: 1;
        unsigned int PrintFormFeed: 1;
        enum DECPES {
            // FullScreen is LinesBuffer+Lines
            // ScrollingRegion is Lines
            FullScreen, ScrollingRegion
        };
        unsigned int PrintExtent: 1;
        //
        VT102Modes()
            : KeyboardAction(Unlocked)
            , InsertionReplacement(Replace)
            , SendReceive(Off)
            , LineFeedNewLine(LineFeed)
            , CursorKey(Cursor)
            , AnsiVT52(ANSI)
            , Column(C132)
            , Scrolling(Smooth)
            , Screen(Normal)
            , Origin(Absolute)
            , AutoWrap(Off)
            , AutoRepeat(On)
            , PrintFormFeed(Off)
            , PrintExtent(FullScreen)
        {
        }
    };
    VT102Modes mModes;
    // To support feature: disable 'auto repeat'.
    std::map<Upp::dword, bool> mKeyStats;
    bool ShouldIgnoreKey(Upp::dword key);
    // charset support
    int mCharset;
    int mSS; // Shift in/out
};

#endif
