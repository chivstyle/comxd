/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "connimpls/vt200/SerialConnVT220.h"

class SerialConnVT320 : public SerialConnVT220 {
public:
    SerialConnVT320(std::shared_ptr<SerialIo> io);

protected:
    // vt320
    virtual void ProcessDECSASD(const std::string&);
    virtual void ProcessDECSSDT(const std::string&);
    virtual void ProcessDECRQTSR(const std::string&);
    virtual void ProcessDECRQPSR(const std::string&);
    virtual void ProcessDECRQM(const std::string&);
    virtual void ProcessANSIRQM(const std::string&);
    virtual void ProcessDECRPM(const std::string&);
    virtual void ProcessDECRQUPSS(const std::string&);
    virtual void ProcessG1_CS96(const std::string&);
    virtual void ProcessG2_CS96(const std::string&);
    virtual void ProcessG3_CS96(const std::string&);
    // override vt220
    void ProcessDECSCL(const std::string&);
    void ProcessDECSEL(const std::string&);
    void ProcessDECSED(const std::string&);
    void ProcessDA(const std::string&);
    void ProcessSecondaryDA(const std::string&);
    // override cs
    void ProcessG0_CS(const std::string&);
    void ProcessG1_CS(const std::string&);
    void ProcessG2_CS(const std::string&);
    void ProcessG3_CS(const std::string&);
    //
private:
	void LoadDefaultModes();
    void InstallFunctions();
};
