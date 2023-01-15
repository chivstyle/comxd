//
// (c) 2020 chiv
//
#pragma once

#include "SerialIo.h"
#include <Core/SSH/SSH.h>
#include <CtrlLib/CtrlLib.h>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

class SSHPort : public SerialIo {
public:
    SSHPort(std::shared_ptr<Upp::SshSession> session, Upp::String host, int port, Upp::String user, Upp::String term = "vt100");
    virtual ~SSHPort();
    static const char* kDeviceType;
    //
    bool Start();
    void Stop();
    //
    Upp::SshShell* CreateShell();
    //
    int Available() const;
    size_t Read(unsigned char* buf, size_t sz);
    size_t Write(const unsigned char* buf, size_t sz);
    std::string DeviceName() const { return "SSH/" + mHost.ToStd(); }
    std::string DeviceType() const { return kDeviceType; }
    // in chars
    void SetConsoleSize(const Upp::Size& csz);
    //
    Upp::SshSession* Session() const { return mSession.get(); }
    Upp::String Host() const { return mHost; }
    Upp::String Term() const { return mTerm; }
    Upp::String User() const { return mUser; }
    int Port() const { return mPort; }
    //
    void Upload();
    void Download();
    //
private:
    SshShell* mShell;
    std::shared_ptr<Upp::SshSession> mSession;
    std::string mName;
    Upp::String mHost;
    Upp::String mTerm;
    Upp::String mUser;
    int mPort;
    std::condition_variable mCondRead;
    std::condition_variable mCondWrite;
    mutable std::mutex mLock;
    std::vector<unsigned char> mRxBuffer;
    volatile bool mShouldExit;
    volatile bool mRunning;
    std::thread mJob;
};