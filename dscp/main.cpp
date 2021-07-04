#include "dscp.h"

dscp::dscp(const String& url, const String& src_filename, const String& dst_filename)
    : mScp(nullptr)
{
    mProgressBar.Set(0);
    CtrlLayout(*this, "SCP");
    // Init
    if (mSession.Timeout(1000).Connect(url)) {
        mScp = new Scp(mSession);
        mCo& [=]() {
            mScp->WhenProgress = [=](int64 actual, int64 total) -> bool {
                PostCallback([=] { mProgressBar.Set((int)actual, (int)total); });
                return false;
            };
            FileOut out(dst_filename);
            bool ok = mScp->LoadFile(out, src_filename);
            PostCallback([=] { this->Break(ok ? 0 : -1); });
        };
        WhenClose = [=]() {
            mScp->Abort();
        };
    }
}

dscp::~dscp()
{
    mCo.Finish();
    delete mScp;
}

GUI_APP_MAIN
{
    const Vector<String>& args = Upp::CommandLine();
    if (args.size() >= 3) {
        dscp(args[0], args[1], args[2]).Run();
    }
#if 0
	else {
		dscp("chiv:1@192.168.229.128:22", "/tmp/system-log.tgz", "d:\\Temp\\tmp.raw").Run();
	}
#endif
}
