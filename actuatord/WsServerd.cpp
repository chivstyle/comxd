//
// (c) 2021 chiv
//
#include "WsServerd.h"

#define TAG "<WsServerd>:"

using namespace Upp;

WsServerd::Worker::Worker()
{
    Ws.NonBlocking();
}

void WsServerd::Worker::Do(std::function<void(const Upp::String&)> on_message)
{
    String text = Ws.Receive();
    if (!text.IsEmpty()) {
        on_message(text);
    }
}

WsServerd::WsServerd(int port)
    : mPort(port)
{
}

WsServerd::WsServerd(const Value& conf)
{
	mPort = conf["ListenPort"];
	//
	LOG(TAG << "listen port " << mPort);
}

bool WsServerd::SendText(const String& text)
{
    bool ok = false;
    mLock.Enter();
    for (int i = mWorkers.GetCount() - 1; i >= 0; i--) {
        if (mWorkers[i].Ws.IsOpen()) {
            mWorkers[i].Ws.SendText(text);
        }
        ok = ok || !mWorkers[i].Ws.IsError();
    }
    mLock.Leave();
    return ok;
}

void WsServerd::Run(volatile bool* should_exit)
{
    TcpSocket server;
    // default values: listen_count:5, ipv6:false, reuse:true, just what we need.
    server.Timeout(0);
    server.Listen(mPort);
    //
    while (*should_exit == false) {
        SocketWaitEvent sel; // select indeed
        sel.Add(server, WAIT_READ);
        // other thread will never add/remove items to/from array, so
        // we need not lock/unlock here
        for (auto& w : mWorkers) {
            w.Ws.AddTo(sel);
        }
        sel.Wait(20);
        mLock.Enter();
        for (int i = mWorkers.GetCount() - 1; i >= 0; i--) {
            if (sel[i+1]) {
                Worker& w = mWorkers[i];
                w.Do([&](const String& message) {
                    WhenMessage(w.Ws, message);
                });
                if (w.Ws.IsClosed() || w.Ws.IsError()) {
                    WhenClosed(w.Ws);
                    mWorkers.Remove(i);
                    LOG(TAG << "WebSocket was closed");
                }
            }
        }
        mLock.Leave();
        if (sel[0]) {
            // add, we should lock and go
            mLock.Enter();
            if (!mWorkers.Add().Ws.NonBlocking().Accept(server)) {
                mWorkers.Drop();
            } else {
                LOG(TAG << "Accept new client");
            }
            mLock.Leave();
        }
    }
}
