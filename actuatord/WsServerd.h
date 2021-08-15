//
// (c) 2021 chiv
//
#ifndef _controllerd_WsServerd_h_
#define _controllerd_WsServerd_h_

#include <Core/Core.h>
#include <functional>

// websocket server
class WsServerd {
public:
    WsServerd(int port);
    WsServerd(const Upp::Value& conf);
    
    void Run(volatile bool* should_exit);
    
    // Send text to all clients
    // If any send was succeeded, return true
    bool SendText(const Upp::String&);
    //
    Upp::Event<Upp::WebSocket&, const Upp::String&> WhenMessage;
    //
    Upp::Event<Upp::WebSocket&> WhenClosed;
    
private:
    int mPort;
    //-----------------------------------------------------------
    class Worker {
    public:
        Worker();
        Upp::WebSocket Ws;
        void Do(std::function<void(const Upp::String&)> on_message);
    };
    Upp::Array<Worker> mWorkers;
    Upp::Mutex mLock;
    //-----------------------------------------------------------
};

#endif
