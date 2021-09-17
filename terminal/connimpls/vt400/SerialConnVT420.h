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
    // override vt320
    void ProcessDECSCL(const std::string_view& p);
    void ProcessDA(const std::string_view&);
    void ProcessSecondaryDA(const std::string_view&);
    void ProcessDECDSR(const std::string_view& p);
    // vt400
    virtual void ProcessTertiaryDA(const std::string_view&);
    virtual void ProcessDECBI(const std::string_view&);
    virtual void ProcessDECFI(const std::string_view&);
    virtual void ProcessDECDC(const std::string_view&);
    virtual void ProcessDECIC(const std::string_view&);
    // override cs
    void ProcessG1_CS96(const std::string_view&);
    void ProcessG2_CS96(const std::string_view&);
    void ProcessG3_CS96(const std::string_view&);
    //
    void SetHostToS7C();
    void SetHostToS8C();
private:
	void LoadDefaultModes();
    void InstallFunctions();
};