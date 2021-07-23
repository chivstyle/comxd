/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "connimpls/vt500/SerialConnVT520.h"
#include <map>

class SerialConnXterm : public SerialConnVT520 {
public:
    SerialConnXterm(std::shared_ptr<SerialIo> io);

protected:
    virtual void ProcessXTPUSHCOLORS(const std::string_view&);
    virtual void ProcessXTPOPCOLORS(const std::string_view&);
    virtual void ProcessXTSMGRAPHICS(const std::string_view&);
    virtual void ProcessXTHIMOUSE(const std::string_view&);
    virtual void ProcessXTRMTITLE(const std::string_view&);
    virtual void ProcessXTMODKEYS(const std::string_view&);
    virtual void ProcessXTSMPOINTER(const std::string_view&);
    virtual void ProcessXTPUSHSGR(const std::string_view&);
    virtual void ProcessXTPOPSGR(const std::string_view&);
    virtual void ProcessXTVERSION(const std::string_view&);
    virtual void ProcessXTRESTORE(const std::string_view&);
    virtual void ProcessXTSAVE(const std::string_view&);
    virtual void ProcessXTWINOPS(const std::string_view&);
    virtual void ProcessXTSMTITLE(const std::string_view&);
    virtual void ProcessXTCHECKSUM(const std::string_view&);
    virtual void ProcessXTDISABLEMODOPTS(const std::string_view&);
    // override vt520
    void ProcessDECSM(const std::string_view& p);
    void ProcessDECRM(const std::string_view& p);
    void ProcessSD(const std::string_view& p);
    void ProcessSGR(const std::string_view& p);
    void ProcessOSC(const std::string_view& p);
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
    //
private:
	void LoadDefaultModes();
    void InstallFunctions();
};
