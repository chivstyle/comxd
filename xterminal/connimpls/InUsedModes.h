//
// (c) 2022 chiv
//
#pragma once

#include <CtrlLib/CtrlLib.h>
#include <functional>

namespace xvt {
class VTModes;
class SerialConnVT;
class InUsedModes : public WithInUsedModes<Upp::TopWindow> {
public:
    typedef InUsedModes CLASSNAME;
    InUsedModes(SerialConnVT* vt, VTModes* modes, std::function<void()> lock_vt,
        std::function<void()> unlock_vt);
    virtual ~InUsedModes();
    //
private:
	VTModes* mModes;
	SerialConnVT* mVt;
	std::function<void()> mOpsLock;
	std::function<void()> mOpsUnlock;
	
    void InstallActions();
};
}
