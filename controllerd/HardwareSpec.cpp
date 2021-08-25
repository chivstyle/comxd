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
	uint8_t  DataType; // 数字模拟判断位
	// 以下为模拟量
	struct Analog_ {
        uint16_t 闸门开度;   // 拉线传感器
    	uint16_t 左吊点速度; // 转速传感器
    	uint16_t 右吊点速度; // 转速传感器
    	uint16_t 燃油箱液位; // 液位传感器
    	uint16_t 水箱液位;   // 液位传感器
    	uint16_t 冷却水温度; // 温度传感器
    	uint16_t 启动电池电量; // 铅酸电池
    	uint16_t 液压油液位; // 液位传感器
    	uint16_t 液压油温度; // 温度传感器
    	uint16_t 左油杆压力; // 压力传感器
    	uint16_t 右油杆压力; // 压力传感器
    	uint16_t 系统压力;   // 压力传感器
    	uint16_t 控制压力;   // 压力传感器
	}
#ifdef __GNUC__
    __attribute__((packed))
#endif
     Analog;
	// 以下为数字量
	struct Digital_ {
    	uint16_t 离合器状态采集;   // 接触开关
    	uint16_t 发动机启动;       // 功率继电器
    	uint8_t 启闭机左吊点速度;  // 转速传感器
    	uint8_t 油门控制开关;      // 油门调节器
    	uint8_t 风门开关;          // 电磁阀
    	uint16_t 手电切换;         // 2位3通电动换向阀
    	uint8_t 电控提升下降;      // 3位4通电动换向阀
    	uint8_t 增容调压[3];       // 5MP, 10MP, 20MP
    	uint16_t 手动止回;         // 电动球阀
    	uint8_t 制动器驱动;        // 电动球阀
    	uint8_t 手动提升下降;      // 3位4通手动换向阀
    	uint16_t 发动机转速;       // 发动机转速
    	uint16_t 燃油温度;         // 燃油温
    	uint8_t 增容档位;          // 增容档位 0,1,2,3
    	uint8_t 工程控制[9];       // 按键
    }
#ifdef __GNUC__
    __attribute__((packed))
#endif
    Digital;
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
        Json("motorStart",  fd.Digital.工程控制[0])
            ("motorStop",   fd.Digital.工程控制[1])
            ("throttleInc", fd.Digital.工程控制[2])
            ("throttleDec", fd.Digital.工程控制[3])
            ("valveInc",    fd.Digital.工程控制[4])
            ("valveDec",    fd.Digital.工程控制[5])
            ("windStart",   fd.Digital.工程控制[6])
            ("windStop",    fd.Digital.工程控制[7])
    );
    json("info",
        Json("battery2",  10)
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
    auto d = reinterpret_cast<const FrameData*>(s->Data);
    if (d->DataType == 0x01) { // Analog
        fd->Analog = d->Analog;
    } else if (d->DataType == 0x02) { // Digital
        fd->Digital = d->Digital;
    }
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
