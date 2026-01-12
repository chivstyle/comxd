/*!
// (c) 2022 chiv
//
*/
#pragma once

#include "SerialIo.h"
#include "amudp.h"
#include <thread>
#include <mutex>
#include <deque>
#include <memory>

class UdpClient : public SerialIo {
public:
	UdpClient(const Upp::String& host, const Upp::String& local, int listen_port, int dest_port);
	~UdpClient();
	static const char* kDeviceType;
	
	Upp::String Host() const { return mHost; }
	Upp::String Local() const { return mLocal; }
	int DestPort() const { return mDestPort; }
	int ListenPort() const { return mListenPort; }
	
	void Set(const Upp::String& host, const Upp::String& local, int listen_port, int dest_port);
	
	bool Start();
	void Stop();
	bool Reconnect();
	//
	int Available() const;
    size_t Read(unsigned char* buf, size_t sz);
    size_t Write(const unsigned char* buf, size_t sz);
    std::string DeviceName() const;
    std::string DeviceType() const { return kDeviceType; }
    
protected:
	std::shared_ptr<amnet::AmUdp> mUdp;
	Upp::String mHost;
	Upp::String mLocal;
	int mDestPort;
	int mListenPort;
	volatile bool mShouldStop;
	volatile bool mRunning;
	mutable std::mutex mLock;
	std::deque<unsigned char> mRxBuffer;
	std::thread mRx;
	//
    void RxProc();
};
