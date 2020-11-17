//
// (c) 2020 chiv
//
#include "resource.h"
#include "SSHPort.h"

using namespace Upp;

SSHPort::SSHPort(const Upp::String& host, int port, const Upp::String& user, const Upp::String& passwd)
{
	mDeviceName = host;
	if (mSession.Timeout(200).Connect(host, port, user, passwd)) {
		mShell = new SshShell(mSession);
		mShell->Timeout(Null);
		mShell->WhenOutput = [=](const void* out, int out_len) {
			if (out_len > 0) {
				std::lock_guard<std::mutex> _(mLock);
				mRxBuffer.insert(mRxBuffer.end(), (unsigned char*)out, (unsigned char*)out + out_len);
				mCond.notify_one();
			}
		};
		mThr = std::thread([=]() {
			// initial size is 60x34
			mShell->Run("ansi", 100, 34, Null);
		});
	} else {
		throw mSession.GetErrorDesc();
	}
}

SSHPort::~SSHPort()
{
	mShell->Close();
	if (mThr.joinable()) {
		mThr.join();
	}
	delete mShell;
}

void SSHPort::SetConsoleSize(const Size& csz)
{
	mShell->PageSize(csz);
}

size_t SSHPort::Available() const
{
	std::lock_guard<std::mutex> _(mLock);
	return mRxBuffer.size();
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
