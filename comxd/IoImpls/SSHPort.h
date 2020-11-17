//
// (c) 2020 chiv
//
#pragma once

#include "SerialIo.h"
#include <Core/SSH/SSH.h>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

class SSHPort : public SerialIo {
public:
    SSHPort(const Upp::String& host, int port, const Upp::String& user, const Upp::String& passwd);
    virtual ~SSHPort();
    //
    size_t Available() const;
    size_t Read(unsigned char* buf, size_t sz);
    size_t Write(const unsigned char* buf, size_t sz);
    std::string DeviceName() const { return mDeviceName; }
    //
    void SetConsoleSize(const Upp::Size& csz);
private:
	Upp::SshShell* mShell;
	Upp::SshSession mSession;
	std::string mDeviceName;
	std::thread mThr;
	Upp::String mQueue;
	std::condition_variable mCond;
	mutable std::mutex mLock;
	std::vector<unsigned char> mRxBuffer;
	bool mShouldExit;
};
