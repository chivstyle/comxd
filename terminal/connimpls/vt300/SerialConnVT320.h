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
    DECSASD = VT300_SEQ_BEGIN,
    DECSSDT,
    DECRQTSR,
    DECRQPSR,
    DECRQM, // set mode dec private
    ANSIRQM, // set mode ansi
    DECRPM,
    DECRQUPSS,
    virtual void ProcessDECASD(const std::string&);
    virtual void ProcessDECSDT(const std::string&);
    virtual void ProcessDECRQTSR(const std::string&);
    virtual void ProcessDECRQPSR(const std::string&);
    virtual void ProcessDECRQM(const std::string&);
    virtual void ProcessANSIRQM(const std::string&);
    virtual void ProcessDECRPM(const std::string&);
    virtual void ProcessDECRQUPSS(const std::string&);
    // level 3
    enum VT320_OperatingLevel {
        VT300_S7C = VT220_CL_MAX,
        VT300_S8C,
        VT320_CL_MAX
    };
    // override vt220
    void ProcessDECSCL(const std::string&);
    void ProcessDECSEL(const std::string&);
    void ProcessDECSED(const std::string&);
    void ProcessDECSM(const std::string&);
    void ProcessDECRM(const std::string&);
    void ProcessDA(const std::string&);
    void ProcessSecondaryDA(const std::string&);
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
