/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "connimpls/vt500/SerialConnVT520.h"
#include <map>

namespace xvt {

class SerialConnXterm : public SerialConnVT520 {
public:
    SerialConnXterm(std::shared_ptr<SerialIo> io);

protected:
    virtual void ProcessXTPUSHCOLORS(const std::string&);
    virtual void ProcessXTPOPCOLORS(const std::string&);
    virtual void ProcessXTSMGRAPHICS(const std::string&);
    virtual void ProcessXTHIMOUSE(const std::string&);
    virtual void ProcessXTRMTITLE(const std::string&);
    virtual void ProcessXTMODKEYS(const std::string&);
    virtual void ProcessXTSMPOINTER(const std::string&);
    virtual void ProcessXTPUSHSGR(const std::string&);
    virtual void ProcessXTPOPSGR(const std::string&);
    virtual void ProcessXTVERSION(const std::string&);
    virtual void ProcessXTRESTORE(const std::string&);
    virtual void ProcessXTSAVE(const std::string&);
    virtual void ProcessXTWINOPS(const std::string&);
    virtual void ProcessXTSMTITLE(const std::string&);
    virtual void ProcessXTCHECKSUM(const std::string&);
    virtual void ProcessXTDISABLEMODOPTS(const std::string&);
    virtual void ProcessXTOSC(const std::string&);
    // override vt520
    void ProcessDECSM(const std::string& p);
    void ProcessDECRM(const std::string& p);
    void ProcessSD(const std::string& p);
    void ProcessSGR(const std::string& p);
    bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
    // override Paste to support bracketed paste.
    void Paste();
    void LeftDown(Upp::Point p, Upp::dword keyflags);
    void RightDown(Upp::Point p, Upp::dword keyflags);
    void MiddleDown(Upp::Point p, Upp::dword keyflags);
    void MouseWheel(Upp::Point, int zdelta, Upp::dword);
    void LeftUp(Upp::Point p, Upp::dword keyflags);
    void RightUp(Upp::Point p, Upp::dword keyflags);
    void MiddleUp(Upp::Point p, Upp::dword keyflags);
    void GotFocus();
    void LostFocus();
    // Xterm provides a alternate screen
    ScreenData mAlternateScr;
    // Xterm resources
    std::map<std::string, std::string> mRcs;
private:
	void LoadDefaultModes();
    void InstallFunctions();
};
}
