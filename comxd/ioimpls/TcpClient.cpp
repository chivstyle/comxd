/*!
// (c) 2022 chiv
//
*/
#include "resource.h"
#include "TcpClient.h"
#include "TcpClientDialog.h"

using namespace Upp;


TcpClient::TcpClient(std::shared_ptr<Upp::TcpSocket> tcp, const Upp::String& host, int port)
    : mTcp(tcp)
    , mHost(host)
    , mPort(port)
    , mShouldStop(false)
    , mRunning(false)
{
}

TcpClient::~TcpClient()
{
    Stop();
}

std::string TcpClient::DeviceName() const
{
    return mTcp->GetPeerAddr().ToStd();
}

void TcpClient::Stop()
{
    mShouldStop = true;
    if (mRx.joinable()) {
        mRx.join();
    }
    mTcp->Close();
}

void TcpClient::RxProc()
{
    const size_t kBufferSize = 512;
    std::vector<unsigned char> buffer(kBufferSize);
    while (!mShouldStop) {
        if (mTcp->WaitRead()) {
	        int ret = mTcp->Get(buffer.data(), kBufferSize);
	        if (ret < 0) {
	            break;
	        }
	        std::lock_guard<std::mutex> _(mLock);
	        mRxBuffer.insert(mRxBuffer.end(), buffer.begin(), buffer.begin() + ret);
        } else {
            // yield cpu
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

int TcpClient::Available() const
{
    std::lock_guard<std::mutex> _(mLock);
    return mRunning && mRx.joinable() ? (int)mRxBuffer.size() : -1;
}

size_t TcpClient::Read(unsigned char* buf, size_t sz)
{
    std::lock_guard<std::mutex> _(mLock);
    size_t k;
    for (k = 0; k < sz && k < mRxBuffer.size(); ++k) {
        buf[k] = mRxBuffer[k];
    }
    mRxBuffer.erase(mRxBuffer.begin(), mRxBuffer.begin() + k);
    return k;
}

size_t TcpClient::Write(const unsigned char* buf, size_t sz)
{
	if (mTcp->WaitWrite()) {
	    int ret = mTcp->Put(buf, (int)sz);
	    if (ret < 0) return 0;
	    return ret;
	}
	return 0;
}

bool TcpClient::Start()
{
	if (!mTcp->IsOpen()) {
		TcpClientDialog d;
		d.Reconnect(this);
	}
	mTcp->Timeout(0); // NON BLOCK
	mShouldStop = false;
    mRx = std::thread([=]() { mRunning = true; RxProc(); mRunning = false; });
	
    return true;
}
