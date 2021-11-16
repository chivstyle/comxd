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
protected:
    // override ECMA48
    void ProcessDA(const std::string& p);
    void ProcessCUP(const std::string& p);
    void ProcessHVP(const std::string& p);
    void ProcessSI(const std::string& p);
    void ProcessSO(const std::string& p);
    // VT100 modes, DEC private
    virtual void ProcessDECSM(const std::string& p);
    virtual void ProcessDECRM(const std::string& p);
    // DEC private
    virtual void ProcessDECDSR(const std::string& p);
    virtual void ProcessDECIND(const std::string& p);
    // VT100 specific, DEC private
    // DECID was ignored, according to vt100 specification
    virtual void ProcessDECREQTPARM(const std::string& p);
    virtual void ProcessDECSTBM(const std::string& p);
    virtual void ProcessDECSC(const std::string& p);
    virtual void ProcessDECRC(const std::string& p);
    virtual void ProcessDECALN(const std::string& p);
    virtual void ProcessDECTST(const std::string& p);
    virtual void ProcessDECLL(const std::string& p);
    virtual void ProcessDECKPNM(const std::string&);
    virtual void ProcessDECKPAM(const std::string&);
    // charset
    virtual void ProcessG0_CS(const std::string& p);
    virtual void ProcessG1_CS(const std::string& p);
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
    void LoadDefaultModes();
    void InstallFunctions();
};
