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
    virtual void ProcessDECAC(const std::string_view& p);
    virtual void ProcessDECRQM_ANSI(const std::string_view& p);
    virtual void ProcessDECRQM_DECP(const std::string_view& p);
    virtual void ProcessDECST8C(const std::string_view& p);
    // override vt420
    void ProcessDECSCL(const std::string_view& p);
    void ProcessSecondaryDA(const std::string_view& p);
    void ProcessDA(const std::string_view& p);
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
