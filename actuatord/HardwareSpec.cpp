//
// (c) 2021 chiv
//
#include "HardwareSpec.h"
#include "Hardwared.h"
#include "WsServerd.h"
#include <stdint.h>
#include <thread>
#include <chrono>

#define TAG "<HardwareSpec>:"

// basic parameters
static const double kQueryPeroid = 0.2; // 0.2 seconds

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
    	uint8_t 未定义[4];         // 未定义的
    	uint8_t 工程控制[10];      // 按键
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

static_assert(sizeof(FrameData) == 0x3a, "sizeof(FrameData) should be 0x3a");

static inline String ToJSON(const FrameData& fd)
{
    Json json("info",
        Json("leftPress", fd.Analog.左油杆压力)
            ("systemPress", fd.Analog.系统压力)
            ("rightPress", fd.Analog.右油杆压力)
            ("controlPress", fd.Analog.控制压力)
            ("waterTank", fd.Analog.水箱液位)
            ("fuelTank", fd.Analog.燃油箱液位)
            ("hydraulic", fd.Analog.液压油液位)
            ("speed", 100) // TODO: What's it ?
            ("waterWe", fd.Analog.冷却水温度)
            ("fuelWe", 45) // TODO: What's it ?
            ("hydraulicWe", fd.Analog.液压油温度)
            ("leftPointSpeed", fd.Analog.左吊点速度)
            ("rightPointSpeed", fd.Analog.右吊点速度)
            ("closeValueScale", "") // TODO: what's it ?
            ("closeValue", fd.Analog.闸门开度)
            ("zengValue", 3) // TODO: what's it ?
            ("motorSpeed", 1200) // TODO: what's it ?
            ("battery", fd.Analog.启动电池电量)
            ("motorRun", true) // TODO: true or false, how to define it ?
            ("windRun", true) // TODO: true or false, how to define it ?
            ("windCanRun", true) // TODO: true or false, how to define it ?
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
    mFrameData = new FrameData;
    memset(mFrameData, 0, sizeof(FrameData));
}

HardwareSpec::~HardwareSpec()
{
    delete mFrameData;
}

void HardwareSpec::RunCommand(const Upp::String& req)
{
    Value s = ParseJSON(req);
    // TODO: translate command and send frame to device
    LOG(TAG << "Command=" << req);
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

void HardwareSpec::Query(volatile bool* should_exit)
{
    std::vector<unsigned char> resp;
    // 查询模拟部分
    auto req_1 = Hardwared::MakeFrame(0x01, 0x03, {
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    });
    LOG(TAG << "Send req_1(analog) to device");
    if (mHw->SendFrame(req_1, resp, kTimeout, should_exit)) {
        LOG(TAG << "Received a valid frame from hardware, parse it as analog");
        ParseQueryResult(resp, mFrameData);
    } else {
        LOG(TAG << "Timeout");
    }
    auto req_2 = Hardwared::MakeFrame(0x01, 0x03, {
        0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00
    });
    LOG(TAG << "Send req_2(digital) to device");
    if (mHw->SendFrame(req_2, resp, kTimeout, should_exit)) {
        LOG(TAG << "Received a valid frame from hardware, parse it as digital");
        ParseQueryResult(resp, mFrameData);
    } else {
        LOG(TAG << "Timeout");
    }
    // Report regardless of success or failure
    mWs->SendText(ToJSON(*mFrameData));
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
            Query(should_exit);
        } else {
            std::this_thread::sleep_for(std::chrono::duration<double>(kQueryPeroid - ts));
        }
    }
}
