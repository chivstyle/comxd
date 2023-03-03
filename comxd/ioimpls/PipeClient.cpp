/*!
// (c) 2022 chiv
//
*/
#include "PipeClient.h"

#ifdef _WIN32

using namespace Upp;

const char* NamedPipeClient::kDeviceType = "PipeClient";

NamedPipeClient::NamedPipeClient(const String& pipe_name)
    : mName(pipe_name)
    , mPipe(INVALID_HANDLE_VALUE)
    , mEventIn(NULL)
    , mEventOut(NULL)
    , mPending(true)
    , mShouldStop(false)
{
}

NamedPipeClient::~NamedPipeClient()
{
    Stop();
}

std::string NamedPipeClient::DeviceName() const
{
    return mName.ToStd();
}

void NamedPipeClient::Stop()
{
    mShouldStop = true;
    if (mRx.joinable()) {
        mRx.join();
    }
    if (mEventIn) {
        CloseHandle(mEventIn);
        mEventIn = NULL;
    }
    if (mEventOut) {
        CloseHandle(mEventOut);
        mEventOut = NULL;
    }
    if (mPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(mPipe);
        mPipe = INVALID_HANDLE_VALUE;
    }
}

void NamedPipeClient::RxProc()
{
    const size_t kBufferSize = 512;
    std::vector<unsigned char> buffer(kBufferSize);
    while (!mShouldStop) {
        DWORD cb;
        // start reading
        OVERLAPPED overlapped = {};
        overlapped.hEvent = mEventIn;
        BOOL bret = ReadFile(mPipe, buffer.data(), (DWORD)buffer.size(), &cb, &overlapped);
        if (!bret) {
            DWORD dret = GetLastError();
            if (dret == ERROR_IO_PENDING) {
                while (!mShouldStop) {
                    DWORD dret = WaitForSingleObject(mEventIn, 200);
                    if (dret == WAIT_OBJECT_0)
                        break;
                    dret = GetLastError();
                }
                // get the result
                BOOL bok = GetOverlappedResult(mPipe, &overlapped, &cb, FALSE);
                if (!bok) {
                    // system fault, break the loop, exit the thread
                    break;
                }
            }
        }
        std::lock_guard<std::mutex> _(mLock);
        mRxBuffer.insert(mRxBuffer.end(), buffer.begin(), buffer.begin() + cb);
    }
}

int NamedPipeClient::Available() const
{
    std::lock_guard<std::mutex> _(mLock);
    return mRx.joinable() ? (int)mRxBuffer.size() : -1;
}

size_t NamedPipeClient::Read(unsigned char* buf, size_t sz)
{
    std::lock_guard<std::mutex> _(mLock);
    size_t k;
    for (k = 0; k < sz && k < mRxBuffer.size(); ++k) {
        buf[k] = mRxBuffer[k];
    }
    mRxBuffer.erase(mRxBuffer.begin(), mRxBuffer.begin() + k);
    return k;
}

size_t NamedPipeClient::WriteOverlapped(const unsigned char* buf, size_t blksz)
{
    OVERLAPPED overlapped = {};
    overlapped.hEvent = mEventOut;
    DWORD cb;
    BOOL bok = WriteFile(mPipe, buf, (DWORD)blksz, &cb, &overlapped);
    if (!bok) {
        DWORD err = GetLastError();
        while (!mShouldStop && err == ERROR_IO_PENDING) {
            DWORD wret = WaitForSingleObject(mEventOut, 200);
            if (wret == WAIT_OBJECT_0)
                break;
            err = GetLastError();
        }
        bok = GetOverlappedResult(mPipe, &overlapped, &cb, FALSE);
        if (!bok) {
            cb = 0; // return 0
        }
    }
    return cb;
}

size_t NamedPipeClient::Write(const unsigned char* buf, size_t sz)
{
    const size_t kBlkSize = 128;
    size_t p = 0;
    while (p < sz) {
        size_t wb = sz - p > (size_t)kBlkSize ? kBlkSize : sz - p;
        size_t cb = WriteOverlapped(buf + p, wb);
        if (cb == 0) {
            break;
        }
        p += cb;
    }
    return p;
}

bool NamedPipeClient::Start()
{
    Vector<char16> dname = ToUtf16(mName); dname.push_back(0);
    // try
    for (int k = 0; k < 3; ++k) {
	    mPipe = CreateFileW(
	         (const WCHAR*)dname.begin(),   // pipe name
	         GENERIC_READ |  // read and write access
	         GENERIC_WRITE,
	         0,              // no sharing
	         NULL,           // default security attributes
	         OPEN_EXISTING,  // opens existing pipe
	         FILE_FLAG_OVERLAPPED,   // default attributes
	         NULL);          // no template file
	    if (mPipe != INVALID_HANDLE_VALUE) {
	        break;
	    }
	    Sleep(100);
    }
    if (mPipe == INVALID_HANDLE_VALUE) return false;
    // we need to wait for the clients
    mEventIn = CreateEvent(NULL, TRUE, TRUE, NULL);
    mEventOut = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (mEventIn == NULL || mEventOut == NULL)
        return false;
    
    mShouldStop = false;
    mRx = std::thread([=]() { RxProc(); });
    
    return true;
}

bool NamedPipeClient::Reconnect()
{
    Stop();
    return Start();
}

#endif
