//
// (c) 2020 chiv
//
#pragma once

#include "Conn.h"
#include "TcpClient.h"
// device
#include <memory>
#include <queue>
//
class TcpClientDialog : public WithTcpClient<TopWindow> {
public:
    typedef TcpClientDialog CLASSNAME;

    TcpClientDialog();
    // create a serialconn from current settings.
    SerialConn* RequestConn();
    bool Reconnect(TcpClient* sp);
    //
protected:
    bool Key(Upp::dword key, int count) override;
    //
private:
    struct TcpClientInfo {
        Upp::String Host;
        int         Port;
    };
    std::deque<TcpClientInfo> mRecents;
    //
    std::deque<TcpClientInfo> GetRecentTcpClientInfos(int count) const;
    void AddRecentTcpClientInfo(const Upp::String& host, int port);
    void SaveRecentTcpClientInfos() const;
    const TcpClientInfo* FindRecent(const Upp::String& host) const;
    const TcpClientInfo* Find(const std::deque<TcpClientInfo>& infos, const Upp::String& host) const;
    //
    SerialConn* mConn;
    void CreateConn();
};
