//
// (c) 2022 chiv
//
#pragma once

#include <CtrlLib/CtrlLib.h>
#include <functional>

namespace xvt {
class VTModes;
class InUsedModes : public WithInUsedModes<Upp::TopWindow> {
public:
    typedef InUsedModes CLASSNAME;
    InUsedModes(VTModes* modes, std::function<void()> lock_vt,
        std::function<void()> unlock_vt);
    virtual ~InUsedModes();
    //
private:
	VTModes* mModes;
	std::function<void()> mOpsLock;
	std::function<void()> mOpsUnlock;
	
    void InstallActions();
};
}
