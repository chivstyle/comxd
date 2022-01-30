/*!
// (c) 2022 chiv
//
*/
#ifdef _WIN32

#include "SerialIo.h"
#include <thread>
#include <mutex>
#include <deque>
#include <Windows.h>

class NamedPipeClient : public SerialIo {
public:
	NamedPipeClient(const Upp::String& pipe_name);
	~NamedPipeClient();
	
	bool Start();
	void Stop();
	int Available() const;
    size_t Read(unsigned char* buf, size_t sz);
    size_t Write(const unsigned char* buf, size_t sz);
    std::string DeviceName() const;
    
protected:
    void RxProc();

private:
	HANDLE mPipe;
	Upp::String mName;
	HANDLE mEventIn;
	HANDLE mEventOut;
	bool mPending;
	std::thread mRx;
	mutable std::mutex mLock;
	std::deque<unsigned char> mRxBuffer;
	volatile bool mShouldStop;
	//
	size_t WriteOverlapped(const unsigned char* buf, size_t blksz);
};

#endif
