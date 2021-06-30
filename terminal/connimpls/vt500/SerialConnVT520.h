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
    // override vt420
    void ProcessDECSCL(const std::string_view& p);
    void ProcessSecondaryDA(const std::string_view& p);
    void ProcessDECSM(const std::string_view& p);
    void ProcessDECRM(const std::string_view& p);
    void ProcessSM(const std::string_view& p);
    void ProcessRM(const std::string_view& p);
    // level 5
    enum VT520_OperatingLevel {
        VT500_S7C = VT420_CL_MAX,
        VT500_S8C,
        VT520_CL_MAX
    };
    //
    struct VT520Modes {
        uint32_t DECNCSM: 1; // no clear screen on column change
        uint32_t DECRLCM: 1; // right to left copy
        uint32_t DECCRTSM: 1; // CRT save mode
        uint32_t DECARSM: 1; // auto resize
        uint32_t DECMCM: 1; // modem control mode
        uint32_t DECAAM: 1; // auto answerback mode
        uint32_t DECCANSM: 1; // conceal answerback message
        uint32_t DECNULM: 1; // ignore null mode
        uint32_t DECHDPXM: 1; // half-duplex mode
        uint32_t DECESKM: 1; // enable secondary keyboard language mode
        uint32_t DECOSCNM: 1; // overscan mode
        uint32_t DECNUMLK: 1; // num lock mode
        uint32_t DECCAPSLK: 1; // caps lock mode
        uint32_t DECKLHIM: 1; // keyboard LED's host indicator mode
        uint32_t DECFWM: 1; // framed windows mode
        uint32_t DECRPL: 1; // review previous lines
        uint32_t DECHWUM: 1; // host wake-up mode
        uint32_t DECATCUM: 1; // alternate text color underline mode
        uint32_t DECATCBM: 1; // alternate text color blink mode
        uint32_t DECBBSM: 1; // bold and blink style mode
        uint32_t DECECM: 1; // erase color mode
        VT520Modes()
        {
        }
    };
    VT520Modes mModes;
    //
private:
    void InstallFunctions();
};
