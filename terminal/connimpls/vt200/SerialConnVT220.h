/*!
// (c) 2021 chiv
//
// Does not support DECUDK,DECDLD
*/
#pragma once

#include "connimpls/vt100/SerialConnVT100.h"

class SerialConnVT220 : public SerialConnVT100 {
public:
    SerialConnVT220(std::shared_ptr<SerialIo> io);
    //
protected:
    // override ecma48
    void ProcessSS2(const std::string_view&);
    void ProcessSS3(const std::string_view&);
    void ProcessLS2(const std::string_view&);
    void ProcessLS3(const std::string_view&);
    void ProcessLS1R(const std::string_view&);
    void ProcessLS2R(const std::string_view&);
    void ProcessLS3R(const std::string_view&);
    // override vt100
    void ProcessDECSM(const std::string_view&);
    void ProcessDECRM(const std::string_view&);
    void ProcessDECDSR(const std::string_view&);
    void ProcessDECSC(const std::string_view&);
    void ProcessDECRC(const std::string_view&);
    void ProcessG0_CS(const std::string_view&);
    void ProcessG1_CS(const std::string_view&);
    // vt200
    virtual void ProcessDECSCL(const std::string_view&);
    virtual void ProcessG2_CS(const std::string_view&);
    virtual void ProcessG3_CS(const std::string_view&);
    virtual void ProcessS7C1T(const std::string_view&);
    virtual void ProcessS8C1T(const std::string_view&);
    virtual void ProcessDECSCA(const std::string_view&);
    virtual void ProcessDECSEL(const std::string_view&);
    virtual void ProcessDECSED(const std::string_view&);
    virtual void ProcessDECSTR(const std::string_view&);
    virtual void ProcessSecondaryDA(const std::string_view&);
    // override
    uint32_t RemapCharacter(uint32_t uc, int charset);
    //
    enum VT220_OperatingLevel {
        VT100,
        VT200_S7C,
        VT200_S8C,
        VT220_CL_MAX
    };
    int mOperatingLevel;
    // key
    virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
    // VT220 cursor data
    struct CursorDataVT220 : public CursorDataVT100 {
        uint32_t   SelectiveErase : 1;
    };
    void SaveCursorData(CursorDataVT220& cd);
    void LoadCursorData(const CursorDataVT220& cd);
    // stands for GR
    int  mExtendedCharset;
    //
    struct VT220Modes {
        enum DECNRCMValue {
            DECNRCM_Multinational = 0,
            DECNRCM_National
        };
        uint32_t DECNRCM : 1;
    };
    VT220Modes mModes;
    CursorDataVT220 mCursorData;
private:
    void InstallFunctions();
};
