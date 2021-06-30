/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "connimpls/vt500/SerialConnVT520.h"

class SerialConnXterm : public SerialConnVT520 {
public:
    SerialConnXterm(std::shared_ptr<SerialIo> io);
    
protected:
    // override vt520
    void ProcessDECSM(const std::string_view& p);
    void ProcessDECRM(const std::string_view& p);
    // Xterm provides a alternate screen
    ScreenData mAlternateScr;
    //
    enum ScreenType {
        SC_Main,
        SC_Alternate
    };
    int mCurrentScr;
    
private:
    void InstallFunctions();
};
