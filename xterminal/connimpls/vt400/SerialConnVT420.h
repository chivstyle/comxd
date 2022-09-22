/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "connimpls/vt300/SerialConnVT320.h"

class SerialConnVT420 : public SerialConnVT320 {
public:
    SerialConnVT420(std::shared_ptr<SerialIo> io);

protected:
	// vt400
    virtual void ProcessTertiaryDA(const std::string&);
    virtual void ProcessDECBI(const std::string&);
    virtual void ProcessDECFI(const std::string&);
    virtual void ProcessDECDC(const std::string&);
    virtual void ProcessDECIC(const std::string&);
    // override vt320
    void ProcessDECSCL(const std::string& p) override;
    void ProcessDA(const std::string&) override;
    void ProcessSecondaryDA(const std::string&) override;
    void ProcessDECDSR(const std::string& p) override;
    // override cs
    void ProcessG1_CS96(const std::string&) override;
    void ProcessG2_CS96(const std::string&) override;
    void ProcessG3_CS96(const std::string&) override;
    //
    void SetHostToS7C();
    void SetHostToS8C();
private:
	void LoadDefaultModes();
    void InstallFunctions();
};
