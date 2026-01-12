/*!
// (c) 2022 chiv
//
*/
#include "resource.h"
#include "UdpClient.h"
#include "UdpClientDialog.h"

using namespace Upp;

const char* UdpClient::kDeviceType = "UdpClient";

UdpClient::UdpClient(const Upp::String& host, const Upp::String& local, int listen_port, int dest_port)
    : mHost(host)
    , mLocal(local)
    , mListenPort(listen_port)
    , mDestPort(dest_port)
    , mShouldStop(false)
    , mRunning(false)
{
}

void UdpClient::Set(const Upp::String& host, const Upp::String& local, int listen_port, int dest_port)
{
	mHost = host;
	mLocal = local;
	mListenPort = listen_port;
	mDestPort = dest_port;
}

UdpClient::~UdpClient()
{
    Stop();
}

std::string UdpClient::DeviceName() const
{
    std::string host = mHost.ToStd();
    host += ":" + std::to_string(mDestPort);
    return host;
}

void UdpClient::Stop()
{
    mShouldStop = true;
    if (mRx.joinable()) {
        mRx.join();
    }
    mRunning = false;
    // delete
    mUdp = nullptr;
}

void UdpClient::RxProc()
{
    const size_t kBufferSize = 65536;
    std::vector<unsigned char> buffer(kBufferSize);
    while (!mShouldStop) {
        int ret = mUdp->Read(buffer.data(), (int)buffer.size(), 100);
        if (ret > 0) {
            std::lock_guard<std::mutex> _(mLock);
	        mRxBuffer.insert(mRxBuffer.end(), buffer.begin(), buffer.begin() + ret);
        }
    }
    //
    mRunning = false;
}

int UdpClient::Available() const
{
    std::lock_guard<std::mutex> _(mLock);
    return (int)mRxBuffer.size();
}

size_t UdpClient::Read(unsigned char* buf, size_t sz)
{
    std::lock_guard<std::mutex> _(mLock);
    size_t k;
    for (k = 0; k < sz && k < mRxBuffer.size(); ++k) {
        buf[k] = mRxBuffer[k];
    }
    mRxBuffer.erase(mRxBuffer.begin(), mRxBuffer.begin() + k);
    return k;
}

size_t UdpClient::Write(const unsigned char* buf, size_t sz)
{
	if (mUdp) {
	    uint32_t dest_ip = amnet::AmUdp::IpAddr(mHost.ToStd().c_str());
	    int ret = mUdp->Write(buf, (int)sz, dest_ip, mDestPort);
	    
	    return ret < 0 ? 0 : ret;
	} else {
		return 0;
	}
}
bool UdpClient::Start()
{
    if (mRunning || mRx.joinable()) return true;
    //
	if (mListenPort == 0) {
		mUdp = std::make_shared<amnet::AmUdp>();
	} else {
	    mUdp = std::make_shared<amnet::AmUdp>(mLocal.ToStd().c_str(), mListenPort);
	}
	mUdp->SetBroadcast(true);
	//
	mShouldStop = false;
	//
	mRunning = true;
	//
    mRx = std::thread([=]() { RxProc(); });
	//
    return true;
}

bool UdpClient::Reconnect()
{
	UdpClientDialog d;
	if (d.RequestReconnect(this)) {
	    Stop();
	    if (d.Reconnect(this)) {
	        return Start();
	    }
	}
	return false;
}
