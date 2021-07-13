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
    // override vt220
    void ProcessDECSCL(const std::string_view&);
    void ProcessDECSEL(const std::string_view&);
    void ProcessDECSED(const std::string_view&);
    void ProcessDECSM(const std::string_view&);
    void ProcessDECRM(const std::string_view&);
    void ProcessDA(const std::string_view&);
    void ProcessSecondaryDA(const std::string_view&);
    // override cs
    void ProcessG0_CS(const std::string_view&);
    void ProcessG1_CS(const std::string_view&);
    void ProcessG2_CS(const std::string_view&);
    void ProcessG3_CS(const std::string_view&);
    //
    struct VT320Modes {
        enum DECKBUMValue {
            DECKBUM_Typewriter = 0,
            DECKBUM_DataProcessing
        };
        uint32_t DECKBUM : 1; // keyboard usage
        VT320Modes()
            : DECKBUM(DECKBUM_DataProcessing)
        {
        }
    };
    VT320Modes mModes;
    //
private:
    void InstallFunctions();
};
