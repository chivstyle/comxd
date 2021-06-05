#ifndef _dscp_dscp_h
#define _dscp_dscp_h

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#define LAYOUTFILE <dscp/dscp.lay>
#include <CtrlCore/lay.h>
#include <Core/SSH/SSH.h>

class dscp : public WithdscpLayout<TopWindow> {
public:
	dscp(const String& uri, const String& src_filename, const String& dst_filename);
	~dscp();
	//
private:
	SshSession mSession;
	Scp* mScp;
	CoWorkNX mCo;
};

#endif
