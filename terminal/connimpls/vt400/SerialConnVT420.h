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
    void ProcessDECSM(const std::string_view& p);
    void ProcessDECRM(const std::string_view& p);
    void ProcessDA(const std::string_view&);
    void ProcessSecondaryDA(const std::string_view&);
    void ProcessDECDSR(const std::string_view& p);
    // vt400
    virtual void ProcessTertiaryDA(const std::string_view&);
    virtual void ProcessDECBI(const std::string_view&);
    virtual void ProcessDECFI(const std::string_view&);
    //
    void SetHostToS7C();
    void SetHostToS8C();

    struct VT420Modes {
        enum DECKBUMValue {
            DECKPM_Character = 0,
            DECKPM_Position
        };
        uint32_t DECKPM : 1; // key position
        VT420Modes()
            : DECKPM(DECKPM_Character)
        {
        }
    };
    VT420Modes mModes;

private:
    void InstallFunctions();
};
