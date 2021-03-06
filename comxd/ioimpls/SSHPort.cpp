//
// (c) 2020 chiv
//
#include "resource.h"
#include "SSHPort.h"

using namespace Upp;

SSHPort::SSHPort(std::shared_ptr<Upp::SshSession> session, String name, String term)
	: mSession(session)
	, mDeviceName(name)
	, mTerm(term)
	, mShouldExit(false)
{
	mShell = new SshShell(*mSession.get());
	mShell->Timeout(Null);
	mShell->WhenOutput = [=](const void* out, int out_len) {
		if (out_len > 0) {
			std::unique_lock<std::mutex> _(mLock);
			mCondWrite.wait(_, [=]() {
				// limit stream to 16K
				return mRxBuffer.size() < 16384 || mShouldExit;
			});
			if (!mShouldExit) {
				mRxBuffer.insert(mRxBuffer.end(), (unsigned char*)out, (unsigned char*)out + out_len);
				mCondRead.notify_one();
			}
		}
	};
}

SSHPort::~SSHPort()
{
	mShouldExit = true;
	//
    mShell->Close();
    //
    mJob.Finish();
    //
    delete mShell;
}

void SSHPort::Stop()
{
    
}

bool SSHPort::Start()
{
	mShouldExit = false;
	mJob & [=]() {
		mShell->Run(mTerm, 80, 34, Null);
	};
	return true;
}

void SSHPort::SetConsoleSize(const Size& csz)
{
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
	String out((const char*)buf, (int)sz);
	mShell->Send(out); // push to queue
	return sz;
}
