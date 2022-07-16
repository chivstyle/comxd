//
// (c) 2020 chiv
//
#include "resource.h"
#include "SSHPort.h"
#include "SSHDevsDialog.h"

using namespace Upp;

SSHPort::SSHPort(std::shared_ptr<Upp::SshSession> session, Upp::String host, int port, Upp::String user, Upp::String term)
    : mSession(session)
    , mHost(host)
    , mPort(port)
    , mUser(user)
    , mTerm(term)
    , mShouldExit(false)
{
	mShell = CreateShell();
}

SSHPort::~SSHPort()
{
    Stop();
}

Upp::SshShell* SSHPort::CreateShell()
{
	auto shell = new SshShell(*mSession.get());
    shell->Timeout(Null);
    shell->SetReadWindowSize(2048);
    shell->WhenOutput = [=](const void* out, int out_len) {
        if (out_len > 0 && mShouldExit == false) {
            std::unique_lock<std::mutex> _(mLock);
            mCondWrite.wait(_, [=]() {
                // limit stream to 2048
                return mRxBuffer.size() < 2048 || mShouldExit;
            });
            if (!mShouldExit) {
                mRxBuffer.insert(mRxBuffer.end(), (unsigned char*)out, (unsigned char*)out + out_len);
                mCondRead.notify_one();
            }
        }
    };
    shell->WhenWait = [=]() {
        PostCallback([=]() { Ctrl::ProcessEvents(); });
    };
    return shell;
}

void SSHPort::Stop()
{
	if (mShell) {
	    mShouldExit = true;
	    mCondWrite.notify_all();
	    // stop the shell
	    mShell->Timeout(200);
	    mShell->Abort();
	    mShell->Close();
	    // wait for the job
	    if (mJob.joinable()) {
	        mJob.join();
	    }
	    // release the shell
	    delete mShell;
	    mShell = nullptr;
	    //
	    mSession->Disconnect();
	}
}

bool SSHPort::Start()
{
	if (!mJob.joinable()) {
	    mShouldExit = false;
	    if (!mShell) {
	        SSHDevsDialog d;
	        d.Reconnect(this);
	        mShell = CreateShell();
	    }
	    mJob = std::thread([=]() {
	        mShell->Run(mTerm, 80, 34, Null);
	    });
	}
    return true;
}

void SSHPort::SetConsoleSize(const Size& csz)
{
	if (!mShell) return;
    if (mShell->GetPageSize() != csz)
        mShell->PageSize(csz);
}

int SSHPort::Available() const
{
    std::lock_guard<std::mutex> _(mLock);
    return (int)mRxBuffer.size();
}

size_t SSHPort::Read(unsigned char* buf, size_t sz)
{
    std::unique_lock<std::mutex> _(mLock);
    if (mCondRead.wait_for(_, std::chrono::milliseconds(50), [=]() { return !mRxBuffer.empty(); })) {
        size_t minsz = std::min(sz, mRxBuffer.size());
        memcpy(buf, mRxBuffer.data(), minsz);
        mRxBuffer.erase(mRxBuffer.begin(), mRxBuffer.begin() + minsz);
        mCondWrite.notify_one();
        //
        return minsz;
    }
    return 0;
}

size_t SSHPort::Write(const unsigned char* buf, size_t sz)
{
	if (!mShell) return 0;
    String out((const char*)buf, (int)sz);
    mShell->Send(out); // push to queue
    return sz;
}
