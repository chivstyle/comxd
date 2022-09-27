/*!
// (c) 2021 chiv
//
// Does not support DECUDK,DECDLD
*/
#pragma once

#include "connimpls/vt100/SerialConnVT100.h"

namespace xvt {
class SerialConnVT220 : public SerialConnVT100 {
public:
    SerialConnVT220(std::shared_ptr<SerialIo> io);
    //
protected:
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
    // override ecma48
    void ProcessSS2(const std::string&) override;
    void ProcessSS3(const std::string&) override;
    void ProcessLS2(const std::string&) override;
    void ProcessLS3(const std::string&) override;
    void ProcessLS1R(const std::string&) override;
    void ProcessLS2R(const std::string&) override;
    void ProcessLS3R(const std::string&) override;
    // override vt100
    void ProcessDECSM(const std::string&) override;
    void ProcessDECRM(const std::string&) override;
    void ProcessDECDSR(const std::string&) override;
    void ProcessDECSC(const std::string&) override;
    void ProcessDECRC(const std::string&) override;
    void ProcessG0_CS(const std::string&) override;
    void ProcessG1_CS(const std::string&) override;
    // override
    uint32_t RemapCharacter(uint32_t uc, int charset) override;
    // key
    bool ProcessKeyDown(Upp::dword key, Upp::dword flags) override;
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
}
