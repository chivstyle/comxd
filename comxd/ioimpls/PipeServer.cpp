/*!
// (c) 2022 chiv
//
*/
#include "PipeServer.h"

#ifdef _WIN32

using namespace Upp;

NamedPipeServer::NamedPipeServer(const String& pipe_name, int in_buffer_sz, int out_buffer_sz,
    int timeout, int number_of_instances)
    : mName(pipe_name)
    , mTimeout(timeout)
    , mNumberOfInstances(number_of_instances)
    , mPipe(INVALID_HANDLE_VALUE)
    , mEventConnected(NULL)
    , mEventIn(NULL)
    , mEventOut(NULL)
    , mPending(true)
    , mShouldStop(false)
    , mInBufferSize(in_buffer_sz)
    , mOutBufferSize(out_buffer_sz)
{
}

NamedPipeServer::~NamedPipeServer()
{
    Stop();
}

std::string NamedPipeServer::DeviceName() const
{
    return mName.ToStd();
}

void NamedPipeServer::Stop()
{
    mShouldStop = true;
    if (mRx.joinable()) {
        mRx.join();
    }
    if (mEventConnected) {
        CloseHandle(mEventConnected);
        mEventConnected = NULL;
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

void NamedPipeServer::RxProc()
{
    std::vector<unsigned char> buffer(mInBufferSize);
    while (!mShouldStop) {
        // connected ?
        if (mPending) {
            DWORD dret = WaitForSingleObject(mEventConnected, 200);
            if (dret == WAIT_TIMEOUT) continue;
            else if (dret == WAIT_OBJECT_0) {
                mPending = false;
            } else break;
        } else {
            DWORD cb = 0;
            // start reading
            OVERLAPPED overlapped = {};
            overlapped.hEvent = mEventIn;
            BOOL bret = ReadFile(mPipe, buffer.data(), (DWORD)buffer.size(), &cb, &overlapped);
            if (!bret) {
                DWORD dret = GetLastError();
                while (!mShouldStop && dret == ERROR_IO_PENDING) {
                    DWORD dret = WaitForSingleObject(mEventIn, 200);
                    if (dret == WAIT_OBJECT_0)
                        break;
                    dret = GetLastError();
                }
                // get the result
                BOOL bok = GetOverlappedResult(mPipe, &overlapped, &cb, FALSE);
                if (!bok) {
                    cb = 0; // set cb to 0, reconnect later
                }
            }
            if (cb == 0) {
                DisconnectNamedPipe(mPipe);
                Connect();
            } else {
                std::lock_guard<std::mutex> _(mLock);
                mRxBuffer.insert(mRxBuffer.end(), buffer.begin(), buffer.begin() + cb);
            }
        }
    }
}

int NamedPipeServer::Available() const
{
    std::lock_guard<std::mutex> _(mLock);
    return mRx.joinable() ? (int)mRxBuffer.size() : -1;
}

size_t NamedPipeServer::Read(unsigned char* buf, size_t sz)
{
    std::lock_guard<std::mutex> _(mLock);
    size_t k;
    for (k = 0; k < sz && k < mRxBuffer.size(); ++k) {
        buf[k] = mRxBuffer[k];
    }
    mRxBuffer.erase(mRxBuffer.begin(), mRxBuffer.begin() + k);
    return k;
}

size_t NamedPipeServer::WriteOverlapped(const unsigned char* buf, size_t blksz)
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
            cb = 0;
        }
    }
    return cb;
}

size_t NamedPipeServer::Write(const unsigned char* buf, size_t sz)
{
    size_t p = 0;
    while (p < sz) {
        size_t wb = sz - p > (size_t)mOutBufferSize ? mOutBufferSize : sz - p;
        size_t cb = WriteOverlapped(buf + p, wb);
        if (cb == 0)
            break;
        p += cb;
    }
    return p;
}

void NamedPipeServer::Connect()
{
    OVERLAPPED overlapped = {};
    overlapped.hEvent = mEventConnected;
    BOOL connected = ConnectNamedPipe(mPipe, &overlapped);
    if (connected) {
        mPending = false;
    } else {
        switch (GetLastError()) {
        case ERROR_IO_PENDING:
            mPending = true;
            break;
        case ERROR_PIPE_CONNECTED:
            break;
        default:break;
        }
    }
}

bool NamedPipeServer::Start()
{
    Vector<char16> dname = ToUtf16(mName); dname.push_back(0);
    mPipe = CreateNamedPipeW(
            (const WCHAR*)dname.begin(),           // pipe name
            PIPE_ACCESS_DUPLEX |     // read/write access
            FILE_FLAG_OVERLAPPED,    // overlapped mode
            PIPE_TYPE_MESSAGE |      // message-type pipe
            PIPE_READMODE_MESSAGE |  // message-read mode
            PIPE_WAIT,               // blocking mode
            mNumberOfInstances,      // number of instances
            (DWORD)mInBufferSize,    // output buffer size
            (DWORD)mOutBufferSize,   // input buffer size
            mTimeout,                // client time-out
            NULL);                   // default security attributes
    if (mPipe == INVALID_HANDLE_VALUE) {
        return false;
    }
    // we need to wait for the clients
    mEventConnected = CreateEvent(NULL, TRUE, TRUE, NULL);
    mEventIn = CreateEvent(NULL, TRUE, TRUE, NULL);
    mEventOut = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (mEventConnected == NULL || mEventIn == NULL || mEventOut == NULL)
        return false;
    // create a connection
    Connect();
    //
    mShouldStop = false;
    mRx = std::thread([=]() { RxProc(); });
    
    return true;
}

#endif
