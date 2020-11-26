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
    SSHPort(std::shared_ptr<Upp::SshSession> session, Upp::String name, Upp::String term = "vt102");
    virtual ~SSHPort();
    //
    bool Start();
    //
    int Available() const;
    size_t Read(unsigned char* buf, size_t sz);
    size_t Write(const unsigned char* buf, size_t sz);
    std::string DeviceName() const { return mDeviceName; }
    // in chars
    void SetConsoleSize(const Upp::Size& csz);
private:
	Upp::SshShell* mShell;
	std::shared_ptr<Upp::SshSession> mSession;
	std::string mDeviceName;
	Upp::String mTerm;
	std::thread mThr;
	Upp::String mQueue;
	std::condition_variable mCond;
	mutable std::mutex mLock;
	std::vector<unsigned char> mRxBuffer;
	bool mShouldExit;
};
