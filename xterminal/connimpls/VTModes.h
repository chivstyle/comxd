//
// (c) 2021 chiv
//
// All modes used by ECMA-048, DEC Terminals, .etc
//
#pragma once
//
#include <map>

namespace xvt {
// ECMA-048
enum AnsiCompatibleMode {
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
#define CASE_OF_MODE_NAME(mode)   case mode : return #mode
static inline const char* AnsiModeName(int mode)
{
	switch (mode) {
	CASE_OF_MODE_NAME(GATM);
	CASE_OF_MODE_NAME(KAM);
	CASE_OF_MODE_NAME(CRM);
	CASE_OF_MODE_NAME(IRM);
	CASE_OF_MODE_NAME(ERM);
	CASE_OF_MODE_NAME(VEM);
	CASE_OF_MODE_NAME(BDSM);
	CASE_OF_MODE_NAME(DCSM);
	CASE_OF_MODE_NAME(HEM);
	CASE_OF_MODE_NAME(PUM);
	CASE_OF_MODE_NAME(SRM);
	CASE_OF_MODE_NAME(FEAM);
	CASE_OF_MODE_NAME(FETM);
	CASE_OF_MODE_NAME(MATM);
	CASE_OF_MODE_NAME(TTM);
	CASE_OF_MODE_NAME(SATM);
	CASE_OF_MODE_NAME(TSM);
	CASE_OF_MODE_NAME(EBM);
	CASE_OF_MODE_NAME(LNM);
	CASE_OF_MODE_NAME(GRCM);
	CASE_OF_MODE_NAME(ZDM);
	}
	return "Undefined ANSI";
}
// DEC private modes, and compatible modes
enum DecpCompatibleMode {
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
static inline const char* DecpModeName(int mode)
{
	switch (mode) {
	CASE_OF_MODE_NAME(DECCKM);
    CASE_OF_MODE_NAME(DECANM);
    CASE_OF_MODE_NAME(DECCOLM);
    CASE_OF_MODE_NAME(DECSCLM);
    CASE_OF_MODE_NAME(DECSCNM);
    CASE_OF_MODE_NAME(DECOM);
    CASE_OF_MODE_NAME(DECAWM);
    CASE_OF_MODE_NAME(DECARM);
    CASE_OF_MODE_NAME(DECPFF);
    CASE_OF_MODE_NAME(DECPEX);
    CASE_OF_MODE_NAME(DECTCEM);
    CASE_OF_MODE_NAME(DECRLM);
    CASE_OF_MODE_NAME(DECHEBM);
    CASE_OF_MODE_NAME(DECHEM);
    CASE_OF_MODE_NAME(DECTEK);
    CASE_OF_MODE_NAME(DECNRCM);
    CASE_OF_MODE_NAME(DECNAKB);
    CASE_OF_MODE_NAME(DECHCCM);
    CASE_OF_MODE_NAME(DECVCCM);
    CASE_OF_MODE_NAME(DECPCCM);
    CASE_OF_MODE_NAME(DECNKM);
    CASE_OF_MODE_NAME(DECBKM);
    CASE_OF_MODE_NAME(DECKBUM);
    CASE_OF_MODE_NAME(DECLRMM);
    CASE_OF_MODE_NAME(DECXRLMM);
    CASE_OF_MODE_NAME(DECSDM);
    CASE_OF_MODE_NAME(DECNCSM);
    CASE_OF_MODE_NAME(DECRLCM);
    CASE_OF_MODE_NAME(DECCRTSM);
    CASE_OF_MODE_NAME(DECARSM);
    CASE_OF_MODE_NAME(DECMCM);
    CASE_OF_MODE_NAME(DECAAM);
    CASE_OF_MODE_NAME(DECCANSM);
    CASE_OF_MODE_NAME(DECCNULM);
    CASE_OF_MODE_NAME(DECHDPXM);
    CASE_OF_MODE_NAME(DECESKM);
    CASE_OF_MODE_NAME(DECOSCNM);
    CASE_OF_MODE_NAME(DECFWM);
    CASE_OF_MODE_NAME(DECRPL);
    CASE_OF_MODE_NAME(DECHWUM);
    CASE_OF_MODE_NAME(DECATCUM);
    CASE_OF_MODE_NAME(DECATCMB);
    CASE_OF_MODE_NAME(DECBSM);
    CASE_OF_MODE_NAME(DECECM);
    CASE_OF_MODE_NAME(SendXyOnPress);
    CASE_OF_MODE_NAME(ShowToolbar);
    CASE_OF_MODE_NAME(StartBlinkingCursorAt610);
    CASE_OF_MODE_NAME(StartBlinkingCursorRc);
    CASE_OF_MODE_NAME(EnableXorBlinkingCursor);
    CASE_OF_MODE_NAME(ShowScrollbar);
    CASE_OF_MODE_NAME(Allow80_132);
    CASE_OF_MODE_NAME(TurnOnMarginBell);
    CASE_OF_MODE_NAME(ReverseWraparoundMode);
    CASE_OF_MODE_NAME(StartLogging);
    CASE_OF_MODE_NAME(UseAlternateScreen_47);
    CASE_OF_MODE_NAME(SendXyOnPressAndRelease);
    CASE_OF_MODE_NAME(UseHiliteMouseTracking);
    CASE_OF_MODE_NAME(UseCellMotionMouseTracking);
    CASE_OF_MODE_NAME(UseAllMotionMouseTracking);
    CASE_OF_MODE_NAME(SendFocusInAndOut);
    CASE_OF_MODE_NAME(EnableUTF8MouseMode);
    CASE_OF_MODE_NAME(EnableSGRMouseMode);
    CASE_OF_MODE_NAME(EnableAlternateScrollMode);
    CASE_OF_MODE_NAME(ScrollToBottomOnTtyOutput);
    CASE_OF_MODE_NAME(ScrollToBottomOnKeyPress);
    CASE_OF_MODE_NAME(EnableUrxvtMouseMode);
    CASE_OF_MODE_NAME(InterpretMetaKey);
    CASE_OF_MODE_NAME(EnableSpecialModifersForAltAndNumLockKeys);
    CASE_OF_MODE_NAME(SendEscWhenMetaModifiesKey);
    CASE_OF_MODE_NAME(SendDelFromTheEditingKeypadDeleteKey);
    CASE_OF_MODE_NAME(SendEscWhenAltModifiesKey);
    CASE_OF_MODE_NAME(KeepSelectionEvenIfNotHighlighted);
    CASE_OF_MODE_NAME(UseClipboardSelection);
    CASE_OF_MODE_NAME(EnableUrgencyWindowManagerHintWhenCtrlGReceived);
    CASE_OF_MODE_NAME(EnableRaisingOfWindowWhenCtrlGReceived);
    CASE_OF_MODE_NAME(ReuseTheMostRecentDataCopiedToClipboard);
    CASE_OF_MODE_NAME(EnableSwitchingToFromAlternateScreen);
    CASE_OF_MODE_NAME(UseAlternateScreen_1047);
    CASE_OF_MODE_NAME(SaveCursor);
    CASE_OF_MODE_NAME(SaveCursorAndSwitchToAlternateScreenThenClear);
    CASE_OF_MODE_NAME(SetTerminfoFunctionKeyMode);
    CASE_OF_MODE_NAME(SetSunFunctionKeyMode);
    CASE_OF_MODE_NAME(SetHPFunctionKeyMode);
    CASE_OF_MODE_NAME(SetSCOFunctionKeyMode);
    CASE_OF_MODE_NAME(SetLegacyKeyboard);
    CASE_OF_MODE_NAME(SetVT220Keyboard);
    CASE_OF_MODE_NAME(SetBracketedPasteMode);
	}
	return "Undefined DECP";
}

class VTModes {
public:
	VTModes();
	
	void SetAnsiMode(int mode, int val);
	void SetDecpMode(int mode, int val);
	int GetAnsiMode(int mode, int def = -1);
	int GetDecpMode(int mode, int def = -1);
	//
	const std::map<int, int> GetAnsiModes() const { return mAnsiModes; }
	const std::map<int, int> GetDecpModes() const { return mDecpModes; }
	//
private:
	std::map<int, int> mAnsiModes;
	std::map<int, int> mDecpModes;
};

}
