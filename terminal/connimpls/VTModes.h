//
// (c) 2021 chiv
//
// All modes used by ECMA-048, DEC Terminals, .etc
//
#pragma once
//
#include <map>
// ECMA-048
enum AnsiCompatibleModes {
    GATM = 1,
    KAM, // Set - Locked, Reset - Unlocked
    CRM,
    IRM, // Set - Insert, Reset - Replace
    SRTM,
    ERM,
    VEM,
    BDSM,
    DCSM,
    HEM,
    PUM,
    SRM, // Set - Off, Reset - On
    FEAM,
    FETM,
    MATM,
    TTM,
    SATM,
    TSM,
    EBM,
    LNM, // Set - New line, Reset - Linefeed
    GRCM,
    ZDM
};
// DEC private modes, and compatible modes
enum DecpCompatibleModes {
    // VT100
    //--------------------------------------------------------------------
    DECCKM = 1, // Set - Application, Reset - Normal
    DECANM = 2, // Set - ANSI, Reset - VT52
    DECCOLM = 3, // Set - 132 columns, Reset - 80 columns
    DECSCLM = 4, // Set - Smooth, Reset - Jump
    DECSCNM = 5, // Set - Reverse, Reset - Normal
    DECOM = 6, // Set - Relative, Reset - Absolute
    DECAWM = 7, // Set - On, Reset - Off
    DECARM = 8, // Set - On, Reset - Off
    //--------------------------------------------------------------------
    // vt220
    DECPFF = 18, // Set - On, Reset Off
    DECPEX = 19, // Set - Full screen, Reset - Scrolling region
    DECTCEM = 25, // Set - Show cursor, Reset - Hide cursor
    DECRLM = 34,
#define DECHEBM   35 // This value conficts with xterm, so we define it as a macro.
    DECHEM = 36,
    DECTEK = 38, // vt240,
    DECNRCM = 42, // vt220
    DECNAKB = 57,
    DECHCCM = 60,
    DECVCCM = 61,
    DECPCCM = 64,
    DECNKM = 66, // vt320
    DECBKM = 67, // vt340, vt420
    DECKBUM = 68,
    DECLRMM = 69, // vt420
    DECXRLMM = 73,
    DECSDM = 80, // Enable Sixel Scrolling
    DECNCSM = 95, // vt510 and up
    DECRLCM = 96,
    DECCRTSM = 97,
    DECARSM = 98,
    DECMCM = 99,
    DECAAM = 100,
    DECCANSM = 101,
    DECCNULM = 102,
    DECHDPXM = 103,
    DECESKM = 104,
    DECOSCNM = 106,
    DECFWM = 111,
    DECRPL = 112,
    DECHWUM = 113,
    DECATCUM = 114,
    DECATCMB = 115,
    DECBSM = 116,
    DECECM = 117,
    //--------------------------------------------------------------------
    // xterm
    SendXyOnPress = 9,
    ShowToolbar = 10,
    StartBlinkingCursorAt610 = 12, // allow control from seqs
    StartBlinkingCursorRc = 13, // allow control from menu/rc
    EnableXorBlinkingCursor = 14, // allow control from both
    ShowScrollbar = 30, // rxvt
    EnableFontShiftingFunctions = 35, // rxvt
    Allow80_132 = 40, // Allow 80 <-> 132 Mode, xterm
    TurnOnMarginBell = 44, // xterm
    ReverseWraparoundMode = 45, // xterm
    StartLogging = 46, // xterm
    UseAlternateScreen_47 = 47, // xterm, the same to 1047 ?
    SendXyOnPressAndRelease = 1000,
    UseHiliteMouseTracking = 1001,
    UseCellMotionMouseTracking = 1002,
    UseAllMotionMouseTracking = 1003,
    SendFocusInAndOut = 1004,
    EnableUTF8MouseMode = 1005,
    EnableSGRMouseMode = 1006,
    EnableAlternateScrollMode = 1007,
    ScrollToBottomOnTtyOutput = 1010,
    ScrollToBottomOnKeyPress = 1011,
    EnableUrxvtMouseMode = 1015,
    InterpretMetaKey = 1034,
    EnableSpecialModifersForAltAndNumLockKeys = 1035,
    SendEscWhenMetaModifiesKey = 1036,
    SendDelFromTheEditingKeypadDeleteKey = 1037,
    SendEscWhenAltModifiesKey = 1039,
    KeepSelectionEvenIfNotHighlighted = 1040,
    UseClipboardSelection = 1041,
    EnableUrgencyWindowManagerHintWhenCtrlGReceived = 1042,
    EnableRaisingOfWindowWhenCtrlGReceived = 1043,
    ReuseTheMostRecentDataCopiedToClipboard = 1044,
    EnableSwitchingToFromAlternateScreen = 1046,
    UseAlternateScreen_1047 = 1047,
    SaveCursor = 1048,
    SaveCursorAndSwitchToAlternateScreenThenClear = 1049,
    SetTerminfoFunctionKeyMode = 1050,
    SetSunFunctionKeyMode = 1051,
    SetHPFunctionKeyMode = 1052,
    SetSCOFunctionKeyMode = 1053,
    SetLegacyKeyboard = 1060,
    SetVT220Keyboard = 1061,
    SetBracketedPasteMode = 2004
};

class VTModes {
public:
	VTModes();
	
	void SetAnsiMode(int mode, int val);
	void SetDecpMode(int mode, int val);
	int GetAnsiMode(int mode, int def = -1);
	int GetDecpMode(int mode, int def = -1);
	//
private:
	std::map<int, int> mAnsiModes;
	std::map<int, int> mDecpModes;
};

