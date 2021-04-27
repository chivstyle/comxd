//
// (c) 2020 chiv
//
#pragma once

#include "SerialIo.h"
#include <CtrlLib/CtrlLib.h>
#include <Core/SSH/SSH.h>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

class SSHPort : public SerialIo {
public:
    SSHPort(std::shared_ptr<Upp::SshSession> session, Upp::String name, Upp::String term = "vt100");
    virtual ~SSHPort();
    //
    bool Start();
    void Stop();
    //
    int Available() const;
    size_t Read(unsigned char* buf, size_t sz);
    size_t Write(const unsigned char* buf, size_t sz);
    std::string DeviceName() const { return mDeviceName; }
    // in chars
    void SetConsoleSize(const Upp::Size& csz);
    //
private:
	SshShell* mShell;
	std::shared_ptr<Upp::SshSession> mSession;
	std::string mDeviceName;
	Upp::String mTerm;
	std::condition_variable mCond;
	mutable std::mutex mLock;
	std::vector<unsigned char> mRxBuffer;
	Upp::CoWorkNX mJob;
};