/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "connimpls/vt400/SerialConnVT420.h"

class SerialConnVT520 : public SerialConnVT420 {
public:
    SerialConnVT520(std::shared_ptr<SerialIo> io);

protected:
    //
    virtual void ProcessDECAC(const std::string& p);
    virtual void ProcessDECST8C(const std::string& p);
    // override vt320
    void ProcessANSIRQM(const std::string& p) override;
    void ProcessDECRQM(const std::string& p) override;
    // override vt420
    void ProcessDECSCL(const std::string& p) override;
    void ProcessSecondaryDA(const std::string& p) override;
    void ProcessDA(const std::string& p) override;
    // override cs
    void ProcessG0_CS(const std::string&) override;
    void ProcessG1_CS(const std::string&) override;
    void ProcessG2_CS(const std::string&) override;
    void ProcessG3_CS(const std::string&) override;
    void ProcessG1_CS96(const std::string&) override;
    void ProcessG2_CS96(const std::string&) override;
    void ProcessG3_CS96(const std::string&) override;
    //
private:
	void LoadDefaultModes();
    void InstallFunctions();
};
