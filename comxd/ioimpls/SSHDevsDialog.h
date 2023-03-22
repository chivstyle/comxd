//
// (c) 2020 chiv
//
#pragma once

#include "Conn.h"
#include "SSHPort.h"
// device
#include <memory>
#include <queue>
//
class SSHDevsDialog : public WithSSHDevs<TopWindow> {
public:
    typedef SSHDevsDialog CLASSNAME;

    SSHDevsDialog();
    // create a serialconn from current settings.
    SerialConn* RequestConn();
    //
    bool RequestReconnect(SSHPort* sp);
    //
    bool Reconnect(SSHPort* sp);
    //
protected:
    bool Key(Upp::dword key, int count) override;
    //
private:
    struct SSHDevInfo {
        Upp::String Host;
        Upp::String User;
        Upp::String Type;
        Upp::String Code;
        int         Port;
    };
    std::deque<SSHDevInfo> mRecents;
    //
    std::deque<SSHDevInfo> GetRecentSSHDevInfos(int count) const;
    void AddRecentSSHDevInfo(const Upp::String& host, const Upp::String& user, const Upp::String& type, const Upp::String& code, int port);
    void SaveRecentSSHDevInfos() const;
    const SSHDevInfo* FindRecent(const Upp::String& host) const;
    const SSHDevInfo* Find(const std::deque<SSHDevInfo>& infos, const Upp::String& host) const;
    //
    SerialConn* mConn;
    void CreateConn();
};
