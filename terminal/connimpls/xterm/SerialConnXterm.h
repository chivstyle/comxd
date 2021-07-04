/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "connimpls/vt500/SerialConnVT520.h"

class SerialConnXterm : public SerialConnVT520 {
public:
    SerialConnXterm(std::shared_ptr<SerialIo> io);
    
protected:
    virtual void ProcessXTPUSHCOLORS(const std::string_view&);
    virtual void ProcessXTPOPCOLORS(const std::string_view&);
    virtual void ProcessXTSMGRAPHICS(const std::string_view&);
    virtual void ProcessXTHIMOUSE(const std::string_view&);
    virtual void ProcessXTRMTITLE(const std::string_view&);
    virtual void ProcessXTMODKEYS(const std::string_view&);
    virtual void ProcessXTSMPOINTER(const std::string_view&);
    virtual void ProcessXTPUSHSGR(const std::string_view&);
    virtual void ProcessXTPOPSGR(const std::string_view&);
    virtual void ProcessXTVERSION(const std::string_view&);
    virtual void ProcessXTRESTORE(const std::string_view&);
    virtual void ProcessXTSAVE(const std::string_view&);
    virtual void ProcessXTWINOPS(const std::string_view&);
    virtual void ProcessXTSMTITLE(const std::string_view&);
    virtual void ProcessXTCHECKSUM(const std::string_view&);
    // override vt520
    void ProcessDECSM(const std::string_view& p);
    void ProcessDECRM(const std::string_view& p);
    void ProcessSD(const std::string_view& p);
    // override Paste to support bracketed paste.
    void Paste();
    // Xterm provides a alternate screen
    ScreenData mAlternateScr;
    //
    struct XtermModes {
        uint32_t SendXyOnPress : 1; // Send X&Y on button press, X10 xterm
        uint32_t ShowToolbar : 1; // Show toolbar, rxvt
        uint32_t BlinkingCursor : 1; // Start blinking cursor, AT&T 610
        uint32_t XOR : 1; // Enable XOR of Blinking Cursor control sequence and menu.
        uint32_t ShowScrollbar : 1; // Show scrollbar, rxvt
        uint32_t FontShifting : 1; // Enable font-shifting functions, rxvt
        uint32_t Allow80T132 : 1; // Allow 80<->132 Mode, xterm
        uint32_t MoreFix : 1;
        uint32_t UseAlternateScr : 1; // xterm
        uint32_t MarginBell : 1; // Turn on margin bell, xterm
        uint32_t ReverseWrapAround : 1; // Reverse wraparound mode
        uint32_t Logging : 1; // Start logging, xterm
        uint32_t SendXyOnPressAndRelease : 1; // X11 xterm
        uint32_t HiliteMouseTracking : 1; // Use Hilite mouse tracking, xterm
        uint32_t CellMotionMouseTracking : 1; // Use Cell Motion mouse tracking, xterm
        uint32_t AllMotionMouseTracking : 1; // Use All Motion mouse tracking
        uint32_t SendFocusInAndOut : 1; // xterm
        uint32_t Utf8Mouse : 1; // xterm
        uint32_t SgrMouse : 1; // xterm
        uint32_t AlternateScroll : 1; // use alternate scroll mode
        uint32_t ScrollToBottomOnOutput : 1; // scroll to bottom on tty output, rxvt
        uint32_t ScrollToBottomOnKeyPress : 1; // scroll to bottom on key press, rxvt
        uint32_t UrxVtMouse : 1; // Enable urxvt mouse mode
        uint32_t InterpretMetaKey : 1; // Interpret meta key, xterm
        uint32_t SpecModAltAndNumlck : 1; // enable special modifiers for Alt and NumLock keys, xterm
        uint32_t SendEscOnMetaModifiesKey : 1; // Send ESC when Meta modifies a key, xterm
        // ON  - send DEL
        // OFF - send VT220 Remove
        uint32_t SendDelFromKeypadDel : 1; // Send DEL from the editing-keypad DEL key
        uint32_t SendEscOnAltModifiesKey : 1; // Send ESC when Alt modifies a key, xterm
        uint32_t KeepSelection : 1; // Keep selection even if no highlighted, xterm
        uint32_t UseClipboardSelection : 1; // xterm
        uint32_t EnableUrgencyWmHint : 1; // Enable urgency window manager hint when CTRL+G is received
        uint32_t EnableWindowRaising : 1; // Enable raising of window when CTRL+G is received
        uint32_t EnableAlternateScr : 1; // Enable switching to/from alternate screen.
        uint32_t TerminfoFuncionKey : 1;
        uint32_t SunFunctionKey : 1;
        uint32_t HpFunctionKey : 1;
        uint32_t ScoFunctionKey : 1;
        uint32_t LegacyKeyboard : 1;
        uint32_t Vt220Keyboard : 1;
        uint32_t BracketedPaste : 1;
        XtermModes()
            : UseAlternateScr(0)
            , EnableAlternateScr(1)
            , SendFocusInAndOut(1)
            , SendXyOnPressAndRelease(1)
        {
        }
    };
    XtermModes mModes;
private:
    void InstallFunctions();
};
