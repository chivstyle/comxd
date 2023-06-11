//
// (c) 2020 chiv
//
#include "terminal_rc.h"
#include "InUsedModes.h"
#include "VTModes.h"
#include "vt100/SerialConnVT100.h"

using namespace xvt;
//
class ModeOption : public Option {
public:
	using Option::Option;
	
	void SetModeId(int mode_id) { mModeId = mode_id; }
	int GetModeId() const { return mModeId; }
	
private:
	int mModeId;
};
//
InUsedModes::InUsedModes(SerialConnVT* vt, VTModes* modes, std::function<void()> lock_vt,
	std::function<void()> unlock_vt)
    : mVt(vt)
    , mModes(modes)
    , mOpsLock(lock_vt)
    , mOpsUnlock(unlock_vt)
{
    Upp::CtrlLayout(*this);
    Title(t_("Virtual Terminal Modes")).Sizeable();
    //
    this->mSp.AddChild(&mAnsiModes);
    this->mSp.AddChild(&mDecpModes);
    // load a snap
    lock_vt();
    {
	    auto& ansi = modes->GetAnsiModes();
	    mAnsiModes.AddColumn("ANSI Compatible Modes").Ctrls<ModeOption>();
	    int idx = 0;
	    for (auto it = ansi.begin(); it != ansi.end(); ++it) {
	        mAnsiModes.Add();
	        ModeOption* option = static_cast<ModeOption*>(mAnsiModes.GetCtrl(idx++, 0));
	        option->SetLabel(AnsiModeName(it->first));
	        option->Set(it->second);
	        option->SetModeId(it->first);
	        option->WhenAction = [=]() {
	            auto vt100 = dynamic_cast<SerialConnVT100*>(vt);
	            if (vt100) {
	                auto mode = std::to_string(option->GetModeId());
	                option->Get() ? vt100->ProcessSM(mode) : vt100->ProcessRM(mode);
	            }
	        };
	    }
    }
    {
	    auto& decp = modes->GetDecpModes();
	    mDecpModes.AddColumn("DEC Private Compatible Modes").Ctrls<ModeOption>();
	    int idx = 0;
	    for (auto it = decp.begin(); it != decp.end(); ++it) {
	        mDecpModes.Add();
	        ModeOption* option = static_cast<ModeOption*>(mDecpModes.GetCtrl(idx++, 0));
	        option->SetLabel(DecpModeName(it->first));
	        option->SetModeId(it->first);
	        option->Set(it->second);
	        option->WhenAction = [=]() {
	            auto vt100 = dynamic_cast<SerialConnVT100*>(vt);
	            if (vt100) {
	                auto mode = std::to_string(option->GetModeId());
	                option->Get() ? vt100->ProcessDECSM(mode) : vt100->ProcessDECRM(mode);
	            }
	        };
	    }
    }
    unlock_vt();
    InstallActions();
}

InUsedModes::~InUsedModes()
{
}


void InUsedModes::InstallActions()
{
}

