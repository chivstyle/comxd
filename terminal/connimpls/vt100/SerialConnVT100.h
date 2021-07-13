/*!
// (c) 2021 chiv
//
// VT52 seqs were ignored, factory will recognize them, but we do not accept them.
//
*/
#pragma once

#include "connimpls/ecma48/SerialConnEcma48.h"
// vt100, include the typical vt102
class SerialConnVT100 : public SerialConnEcma48 {
public:
    SerialConnVT100(std::shared_ptr<SerialIo> io);
    // override ECMA48
    void ProcessDA(const std::string_view& p);
    void ProcessCUP(const std::string_view& p);
    void ProcessHVP(const std::string_view& p);
    void ProcessSI(const std::string_view& p);
    void ProcessSO(const std::string_view& p);
    // VT100 modes, DEC private
    virtual void ProcessDECSM(const std::string_view& p);
    virtual void ProcessDECRM(const std::string_view& p);
    // DEC private
    virtual void ProcessDECDSR(const std::string_view& p);
    virtual void ProcessDECIND(const std::string_view& p);
    // VT100 specific, DEC private
    // DECID was ignored, according to vt100 specification
    virtual void ProcessDECREQTPARM(const std::string_view& p);
    virtual void ProcessDECSTBM(const std::string_view& p);
    virtual void ProcessDECSC(const std::string_view& p);
    virtual void ProcessDECRC(const std::string_view& p);
    virtual void ProcessDECALN(const std::string_view& p);
    virtual void ProcessDECTST(const std::string_view& p);
    virtual void ProcessDECLL(const std::string_view& p);
    virtual void ProcessDECKPNM(const std::string_view&);
    virtual void ProcessDECKPAM(const std::string_view&);
    // charset
    virtual void ProcessG0_CS(const std::string_view& p);
    virtual void ProcessG1_CS(const std::string_view& p);
    virtual uint32_t RemapCharacter(uint32_t uc, int charset);
    //
    virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
    //
    virtual void SetCursorToHome();
    //
    struct VT100Modes {
        enum DECCKM_Value {
            DECCKM_Cursor = 0,
            DECCKM_Application
        };
        uint32_t DECCKM : 1;
        enum DECANM_Value {
            DECANM_VT52 = 0,
            DECANM_ANSI
        };
        uint32_t DECANM : 1;
        enum DECCOLM_Value {
            DECCOLM_80 = 0,
            DECCOLM_132
        };
        uint32_t DECCOLM : 1;
        enum DECSCLM_Value {
            DECSCLM_Jump = 0,
            DECSCLM_Smooth
        };
        uint32_t DECSCLM : 1;
        enum DECSCNM_Value {
            DECSCNM_Normal = 0,
            DECSCNM_Reverse
        };
        uint32_t DECSCNM : 1;
        enum DECOM_Value {
            DECOM_Absolute = 0,
            DECOM_Relative
        };
        uint32_t DECOM : 1;
        enum BOOL_Value {
            OFF,
            ON
        };
        uint32_t DECAWM : 1;
        uint32_t DECARM : 1;
        uint32_t DECPFF : 1;
        enum DECPEX_Value {
            DECPEX_ScrollingRegion = 0,
            DECPEX_FullScreen
        };
        uint32_t DECPEX : 1;
        // This is not a VT100 mode, but it's proper to place it here.
        uint32_t DECKPM : 1;
        enum {
            DECKPM_PAM = 0,
            DECKPM_PNM
        };
        //
        VT100Modes()
            : DECCKM(DECCKM_Cursor)
            , DECANM(DECANM_ANSI)
            , DECCOLM(DECCOLM_80)
            , DECSCLM(DECSCLM_Smooth)
            , DECSCNM(DECSCNM_Normal)
            , DECOM(DECOM_Absolute)
            , DECAWM(ON)
            , DECARM(OFF) // We ignore auto-repeat mode
            , DECPFF(OFF)
            , DECPEX(DECPEX_FullScreen)
            , DECKPM(DECKPM_PNM)
        {
        }
    };
    VT100Modes mModes;
    // cursor position, graphics rendition, character set
    struct CursorDataVT100 {
        int Vx, Vy;
        int Px, Py;
        int Charset;
        VTStyle Style;
    };
    //
    CursorDataVT100 mCursorData;
    //
    void SaveCursorData(CursorDataVT100& cd);
    void LoadCursorData(const CursorDataVT100& cd);

private:
    void InstallFunctions();
};
