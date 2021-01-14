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
{
	mShell = new SshShell(*mSession.get());
	mShell->Timeout(Null);
	mShell->WhenOutput = [=](const void* out, int out_len) {
		if (out_len > 0) {
			std::lock_guard<std::mutex> _(mLock);
			mRxBuffer.insert(mRxBuffer.end(), (unsigned char*)out, (unsigned char*)out + out_len);
			mCond.notify_one();
		}
	};
	mShell->WhenInput = [=]() {
		std::unique_lock<std::mutex> _(mLock);
		// block the thread for some time to yield cpu if queue is empty.
		mCond.wait_for(_, std::chrono::milliseconds(10), [=]() { return !mQueue.IsEmpty(); });
	};
}

SSHPort::~SSHPort()
{
	try {
		mShell->Close();
	} catch (...) {}
	if (mThr.joinable()) {
		mThr.join();
	}
	delete mShell;
}

bool SSHPort::Start()
{
	mThr = std::thread([=]() {
		try {
			mShell->Run(mTerm, 80, 34, Null);
		} catch (...) {}
	});
	return true;
}

void SSHPort::SetConsoleSize(const Size& csz)
{
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
	if (mCond.wait_for(_, std::chrono::milliseconds(50), [=]() { return !mRxBuffer.empty(); })) {
		size_t minsz = std::min(sz, mRxBuffer.size());
		memcpy(buf, mRxBuffer.data(), minsz);
		mRxBuffer.erase(mRxBuffer.begin(), mRxBuffer.begin() + minsz);
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
