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
    void ProcessSS2(const std::string&);
    void ProcessSS3(const std::string&);
    void ProcessLS2(const std::string&);
    void ProcessLS3(const std::string&);
    void ProcessLS1R(const std::string&);
    void ProcessLS2R(const std::string&);
    void ProcessLS3R(const std::string&);
    // override vt100
    void ProcessDECSM(const std::string&);
    void ProcessDECRM(const std::string&);
    void ProcessDECDSR(const std::string&);
    void ProcessDECSC(const std::string&);
    void ProcessDECRC(const std::string&);
    void ProcessG0_CS(const std::string&);
    void ProcessG1_CS(const std::string&);
    // vt200
    virtual void ProcessDECSCL(const std::string&);
    virtual void ProcessG2_CS(const std::string&);
    virtual void ProcessG3_CS(const std::string&);
    virtual void ProcessS7C1T(const std::string&);
    virtual void ProcessS8C1T(const std::string&);
    virtual void ProcessDECSCA(const std::string&);
    virtual void ProcessDECSEL(const std::string&);
    virtual void ProcessDECSED(const std::string&);
    virtual void ProcessDECSTR(const std::string&);
    virtual void ProcessSecondaryDA(const std::string&);
    // override
    uint32_t RemapCharacter(uint32_t uc, int charset);
    // key
    virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
    // VT220 cursor data
    struct CursorDataVT220 : public CursorDataVT100 {
        uint32_t SelectiveErase : 1;
    };
    void SaveCursorData(CursorDataVT220& cd);
    void LoadCursorData(const CursorDataVT220& cd);
    // stands for GR
    int mExtendedCharset;
    //
    CursorDataVT220 mCursorData;

    void SetOperatingLevel(int level);
    int GetOperatingLevel() const;

private:
    int mOperatingLevel;
    void LoadDefaultModes();
    void InstallFunctions();
};
