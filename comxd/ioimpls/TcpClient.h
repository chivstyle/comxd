/*!
// (c) 2022 chiv
//
*/
#pragma once

#include "SerialIo.h"
#include <thread>
#include <mutex>
#include <deque>
#include <memory>

class TcpClient : public SerialIo {
public:
	TcpClient(std::shared_ptr<Upp::TcpSocket> tcp, const Upp::String& host, int port);
	~TcpClient();
	static const char* kDeviceType;
	
	Upp::String Host() { return mHost; }
	int Port() { return mPort; }
	Upp::TcpSocket* Tcp() const { return mTcp.get(); }
	
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
	std::shared_ptr<Upp::TcpSocket> mTcp;
	Upp::String mHost;
	int mPort;
	volatile bool mShouldStop;
	mutable std::mutex mLock;
	std::deque<unsigned char> mRxBuffer;
	std::thread mRx;
	//
    void RxProc();
};
