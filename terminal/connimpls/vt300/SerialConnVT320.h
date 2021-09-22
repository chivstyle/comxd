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
    virtual void ProcessDECSASD(const std::string_view&);
    virtual void ProcessDECSSDT(const std::string_view&);
    virtual void ProcessDECRQTSR(const std::string_view&);
    virtual void ProcessDECRQPSR(const std::string_view&);
    virtual void ProcessDECRQM(const std::string_view&);
    virtual void ProcessANSIRQM(const std::string_view&);
    virtual void ProcessDECRPM(const std::string_view&);
    virtual void ProcessDECRQUPSS(const std::string_view&);
    virtual void ProcessG1_CS96(const std::string_view&);
    virtual void ProcessG2_CS96(const std::string_view&);
    virtual void ProcessG3_CS96(const std::string_view&);
    // override vt220
    void ProcessDECSCL(const std::string_view&);
    void ProcessDECSEL(const std::string_view&);
    void ProcessDECSED(const std::string_view&);
    void ProcessDA(const std::string_view&);
    void ProcessSecondaryDA(const std::string_view&);
    // override cs
    void ProcessG0_CS(const std::string_view&);
    void ProcessG1_CS(const std::string_view&);
    void ProcessG2_CS(const std::string_view&);
    void ProcessG3_CS(const std::string_view&);
    //
private:
	void LoadDefaultModes();
    void InstallFunctions();
};
