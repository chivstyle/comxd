//
// (c) 2021 chiv
//
#ifndef _controllerd_HardwareSpec_h_
#define _controllerd_HardwareSpec_h_

#include "Core/Core.h"
#include <queue>
#include <mutex>
#include <condition_variable>

struct DeviceStatus;
class Hardwared;
class WsServerd;
class Database;
class HardwareSpec {
public:
    HardwareSpec(Hardwared* hardware, WsServerd* server, Database* db);
    virtual ~HardwareSpec();
    //
    void Run(volatile bool* should_exit);
protected:
    static const int kTimeout = 1000;
    //
    void ProcessRequest(Upp::WebSocket& ws, const Upp::String&);
    // hardware specification
    void RunCommand(Upp::WebSocket& ws, const Upp::String&);
    void Query(volatile bool* should_exit);
    void ParseQueryResult(const std::vector<unsigned char>& frame, DeviceStatus* fd);
    //
private:
    Hardwared* mHw;
    WsServerd* mWs;
    Database* mDb;
    //
    bool mDeviceReady;
    //
    DeviceStatus* mDeviceStatus;
};

#endif
