//
// (c) 2021 chiv
//
#ifndef _controllerd_HardwareSpec_h_
#define _controllerd_HardwareSpec_h_

#include "Core/Core.h"
#include <queue>
#include <mutex>
#include <condition_variable>

class FrameData;
class Hardwared;
class WsServerd;
class Actuator;
class HardwareSpec {
public:
    HardwareSpec(Hardwared* hardware, WsServerd* server);
    virtual ~HardwareSpec();
    //
    void Run(volatile bool* should_exit);
protected:
    static const int kTimeout = 1000;
    //
    void ProcessRequest(const Upp::String&);
    // hardware specification
    void RunCommand(const Upp::String&);
    void Query();
    void ParseQueryResult(const std::vector<unsigned char>& frame, FrameData* fd);
    //
private:
    Hardwared* mHw;
    WsServerd* mWs;
    // command queue
    std::queue<Upp::String> mReqs;
    std::mutex mLock;
    std::condition_variable mCond;
    //
    FrameData* mFrameData;
};

#endif
