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
static const double kQueryPeroid = 0.5; // 0.5 seconds

using namespace Upp;

// 反应设备状态
// Toolchain Hints: 小端、自然对齐
// 成员命名来自web app
// 总是位于Frame的Data区域, 从Data区域的地址0开始
struct ReadonlyStatus {
    uint16_t leftPress; // 左油杆压力
    uint16_t rightPress;
    uint16_t systemPress; // 系统压力
    uint16_t controlPress; // 控制压力
    uint16_t waterTank; // 水箱液位
    uint16_t fuelTank;
    uint16_t hydraulic; // 液压油液位
    uint16_t closeValue; // 闸门开度
    uint16_t speed;
    uint16_t waterWe; // 冷却水温度
    uint16_t fuelWe; // 燃油温度
    uint16_t hydraulicWe; // 液压油温度
    uint16_t leftPointSpeed; // 左吊点速度
    uint16_t rightPointSpeed; // 右吊点速度
    uint16_t motorSpeed; // 发动机转速
    uint16_t battery; // 启动电池电量
};
struct ReadWriteStatus {
    uint8_t windRun;
    uint8_t motorRun;
    uint8_t gearValue; // 发动机档位, 百分比0-100
    uint8_t zengValue; // 0,1,2,3
    uint8_t zengPreset[3]; // 预设的增容量
    uint16_t closeValue; // 0-300 cm
};
struct DeviceStatus {
    struct ReadonlyStatus Ro; // 32 bytes
    struct ReadWriteStatus Rw; // 10 bytes, 其中1个是因为自然对齐补上的空白
}; // Total : 42 bytes

static_assert(sizeof(DeviceStatus) == 42, "sizeof(DeviceStatus) should be 42");

static inline String ToJSON(const DeviceStatus& fd)
{
    Json json(
        "info",
        Json("leftPress", fd.Ro.leftPress)
            ("systemPress", fd.Ro.systemPress)
            ("rightPress", fd.Ro.rightPress)
            ("controlPress", fd.Ro.controlPress)
            ("waterTank", fd.Ro.waterTank)
            ("fuelTank", fd.Ro.fuelTank)
            ("hydraulic", fd.Ro.hydraulic)
            ("speed", fd.Ro.speed)
            ("waterWe", fd.Ro.waterWe)
            ("fuelWe", fd.Ro.fuelWe)
            ("hydraulicWe", fd.Ro.hydraulicWe)
            ("leftPointSpeed", fd.Ro.leftPointSpeed)
            ("rightPointSpeed", fd.Ro.rightPointSpeed)
            ("motorSpeed", fd.Ro.motorSpeed)
            ("battery", fd.Ro.battery)
    );
    JsonArray caps;
    caps << (int)fd.Rw.zengPreset[0] << (int)fd.Rw.zengPreset[1] << (int)fd.Rw.zengPreset[2];
    json("outInfo",
        Json("windRun", fd.Rw.windRun)
            ("motorRun", fd.Rw.motorRun)
            ("zengValue", fd.Rw.zengValue)
            ("closeValue", fd.Rw.closeValue)
            ("gearValue", fd.Rw.gearValue)
            ("zengPreset", caps)
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
    mDeviceStatus = new DeviceStatus;
    memset(mDeviceStatus, 0, sizeof(DeviceStatus));
}

HardwareSpec::~HardwareSpec()
{
    delete mDeviceStatus;
}
// safe, this routine was running in this thread (Run)
void HardwareSpec::RunCommand(const Upp::String& req)
{
    Value s = ParseJSON(req);
#if 0
    {
        closeValue: 0,
        gearValue: 10,
        motorRun: false, //发动机启动状态
        windRun: false, //风机运行状态
        zengValue: 3, //增容设置值,
        zengPreset: []
      }
#endif
    if (!mDeviceReady) {
        LOG(TAG << "Device is not ready, ignore this command");
    } else if (!s.IsNull()) {
        LOG(TAG << "Send req to device = " << req);
        //
        auto fd = mDeviceStatus;
        fd->Rw.closeValue = static_cast<uint16_t>((int)(s["closeValue"]));
        fd->Rw.motorRun = s["motorRun"] ? 1 : 0;
        fd->Rw.windRun = s["windRun"] ? 1 : 0;
        fd->Rw.zengValue = static_cast<uint8_t>((int)s["zengValue"]);
        fd->Rw.gearValue = static_cast<uint8_t>((int)s["gearValue"]);
        for (int k = 0; k < 3 && k < s["zengPreset"].GetCount(); ++k) {
            fd->Rw.zengPreset[k] = static_cast<uint8_t>((int)s["zengPreset"][k]);
        }
        // send command to device
        // we do not care about the response, so set timeout to 0
        std::vector<unsigned char> resp;
        mHw->SendFrame(Hardwared::MakeFrame(1, 1, (const unsigned char*)fd, sizeof(DeviceStatus)), resp, 0);
    }
}

void HardwareSpec::ProcessRequest(const Upp::String& request)
{
    std::lock_guard<std::mutex> _(mLock);
    mReqs.push(request);
    mCond.notify_all();
}

void HardwareSpec::ParseQueryResult(const std::vector<unsigned char>& frame, DeviceStatus* fd)
{
    auto s = reinterpret_cast<const Hardwared::Frame*>(frame.data());
    *fd = *reinterpret_cast<const DeviceStatus*>(s->Data);
}

void HardwareSpec::Query(volatile bool* should_exit)
{
    std::vector<unsigned char> resp;
    // 查询
    auto req_1 = Hardwared::MakeFrame(0x01, 0x03, {
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    });
    if (mHw->SendFrame(req_1, resp, kTimeout, should_exit)) {
        mDeviceReady = true;
        //
        ParseQueryResult(resp, mDeviceStatus);
        //
        mWs->SendText(ToJSON(*mDeviceStatus));
    } else {
        mDeviceReady = false;
    }
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
