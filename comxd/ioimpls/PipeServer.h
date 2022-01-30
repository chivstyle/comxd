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

class NamedPipeServer : public SerialIo {
public:
	NamedPipeServer(const Upp::String& pipe_name, int in_buffer_sz, int out_buffer_sz,
        int timeout, int number_of_instances);
	~NamedPipeServer();
	
	bool Start();
	void Stop();
	int Available() const;
    size_t Read(unsigned char* buf, size_t sz);
    size_t Write(const unsigned char* buf, size_t sz);
    std::string DeviceName() const;
    
protected:
    void RxProc();
    void Connect();
    
private:
	HANDLE mPipe;
	Upp::String mName;
	HANDLE mEventConnected;
	HANDLE mEventIn;
	HANDLE mEventOut;
	int mTimeout;
	int mNumberOfInstances;
	int mInBufferSize;
	int mOutBufferSize;
	bool mPending;
	std::thread mRx;
	mutable std::mutex mLock;
	std::deque<unsigned char> mRxBuffer;
	volatile bool mShouldStop;
	//
	size_t WriteOverlapped(const unsigned char* buf, size_t blksz);
};

#endif
