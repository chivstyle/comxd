//
// (c) 2020 chiv
//
#pragma once

#include "Conn.h"
#include "UdpClient.h"
// device
#include <memory>
#include <queue>
//
class UdpClientDialog : public WithUdpClient<TopWindow> {
public:
    typedef UdpClientDialog CLASSNAME;

    UdpClientDialog();
    // create a serialconn from current settings.
    SerialConn* RequestConn();
    
    bool RequestReconnect(UdpClient* sp);
    bool Reconnect(UdpClient* sp);
    //
protected:
    bool Key(Upp::dword key, int count) override;
    //
private:
    struct UdpClientInfo {
        Upp::String Host;
        Upp::String Local;
        int         DestPort;
        int         ListenPort;
    };
    std::deque<UdpClientInfo> mRecents;
    //
    std::deque<UdpClientInfo> GetRecentUdpClientInfos(int count) const;
    void AddRecentUdpClientInfo(const Upp::String& host, const Upp::String& local, int listen_port, int dest_port);
    void SaveRecentUdpClientInfos() const;
    const UdpClientInfo* FindRecent(const Upp::String& host) const;
    const UdpClientInfo* Find(const std::deque<UdpClientInfo>& infos, const Upp::String& host) const;
    //
    SerialConn* mConn;
    void CreateConn();
};
