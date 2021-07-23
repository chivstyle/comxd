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
    
    enum VT100Modes {
        DECCKM = 1, // Set - Application, Reset - Normal
        DECANM = 2, // Set - ANSI, Reset - VT52
        DECCOLM = 3, // Set - 132 columns, Reset - 80 columns
        DECSCLM = 4, // Set - Smooth, Reset - Jump
        DECSCNM = 5, // Set - Reverse, Reset - Normal
        DECOM = 6, // Set - Relative, Reset - Absolute
        DECAWM = 7, // Set - On, Reset - Off
        DECARM = 8, // Set - On, Reset - Off
        DECPFF = 18, // Set - On, Reset Off
        DECPEX = 19, // Set - Full screen, Reset - Scrolling region
    };
    void SetDecMode(int mode, int val);
    // return -1 if there's no mode found.
    int GetDecMode(int mode, int def = -1);
    
protected:
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
    // Dec keypad mode
    enum KeypadMode {
        KM_Normal = 0,
        KM_Application
    };
    int mKeypadMode;

private:
    std::map<int, int> mDecModes;
    void LoadDefaultModes();
    void InstallFunctions();
};
