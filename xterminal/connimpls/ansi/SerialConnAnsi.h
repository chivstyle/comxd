/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "connimpls/vt100/SerialConnVT100.h"

namespace xvt {

class SerialConnAnsi : public SerialConnVT100 {
public:
    SerialConnAnsi(std::shared_ptr<SerialIo> io);

protected:
    void ProcessSS2(const std::string&);
    void ProcessSS3(const std::string&);
    // override
    uint32_t RemapCharacter(uint32_t uc, int charset);
    //
private:
    void InstallFunctions();
};

}
