//
// (c) 2021 chiv
//
#include "HardwareSpec.h"
#include "Hardwared.h"
#include "WsServerd.h"
#include "Actuator.h"
#include <stdint.h>
#include <thread>
#include <chrono>

using namespace Upp;

#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#endif
struct FrameData {
	uint8_t RS[0x2f];
	uint8_t battery;
	uint8_t Keys[10];
}
#ifdef __GNUC__
__attribute__((packed))
#endif
;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#define TAG "<HardwareSpec>:"

static_assert(sizeof(FrameData) == 0x3a, "sizeof(FrameData) should be 0x3a");

static inline String ToJSON(const FrameData& fd)
{
    Json json("buttons",
        Json("motorStart",  fd.Keys[0])
            ("motorStop",   fd.Keys[1])
            ("throttleInc", fd.Keys[2])
            ("throttleDec", fd.Keys[3])
            ("valveInc",    fd.Keys[4])
            ("valveDec",    fd.Keys[5])
            ("windStart",   fd.Keys[6])
            ("windStop",    fd.Keys[7])
    );
    json("info",
        Json("battery2",  fd.battery)
    );
    return json.ToString();
}

HardwareSpec::HardwareSpec(Hardwared* hardware, WsServerd* server)
    : mHw(hardware)
    , mWs(server)
{
    mWs->WhenMessage = [=](WebSocket& ws, const String& message) {
        (void)ws;
        this->ProcessRequest(message);
    };
    mHw->WhenResponse = [=](const std::vector<unsigned char>& frame) {
        auto f = reinterpret_cast<const Hardwared::Frame*>(frame.data());
        auto d = reinterpret_cast<const FrameData*>(f->Data);
        auto s = ToJSON(*d);
        LOG(TAG << "Buttons = " << s);
        mWs->SendText(s);
    };
    mFrameData = new FrameData;
}

HardwareSpec::~HardwareSpec()
{
    delete mFrameData;
}

void HardwareSpec::RunCommand(const Upp::String& req)
{
    // There's no need to do this.
}

void HardwareSpec::ProcessRequest(const Upp::String& request)
{
    std::lock_guard<std::mutex> _(mLock);
    mReqs.push(request);
    mCond.notify_all();
}

void HardwareSpec::ParseQueryResult(const std::vector<unsigned char>& frame, FrameData* fd)
{
    auto s = reinterpret_cast<const Hardwared::Frame*>(frame.data());
    *mFrameData = *reinterpret_cast<const FrameData*>(s->Data);
}

void HardwareSpec::Query()
{
    // There's no need to query.
}
// Run command committed to command queue, and query status periodically
void HardwareSpec::Run(volatile bool* should_exit)
{
    while (*should_exit == false) {
        // serve the request firstly
        auto t1 = std::chrono::high_resolution_clock::now();
        std::unique_lock<std::mutex> _(mLock);
        bool ret = mCond.wait_for(_, std::chrono::milliseconds(100), [=]() {
            return !mReqs.empty();
        });
        if (ret) {
            String req = mReqs.front();mReqs.pop();
            _.unlock(); // unique lock allows this ops
            //
            RunCommand(req);
        } else _.unlock();
        //
        auto ts = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t1).count();
        if (ts > 0.1) {
            // query
            Query();
        } else {
            std::this_thread::sleep_for(std::chrono::duration<double>(0.1 - ts));
        }
    }
}
