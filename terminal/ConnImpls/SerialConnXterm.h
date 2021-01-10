//
// (c) 2020 chiv
//
#pragma once

#include "Conn.h"
#include <CtrlLib/Terminal/Terminal.h>
// c++11 thread
#include <thread>
#include <vector>
#include <mutex>

class SerialConnXterm : public SerialConn {
public:
    using Superclass = SerialConn;
    SerialConnXterm(std::shared_ptr<SerialIo> serial);
    virtual ~SerialConnXterm();
    //
    bool Start();
    //
protected:
	void Layout();
	void DoLayout();
private:
    Upp::TerminalCtrl mTerm;
    std::mutex mRxLock;
    std::vector<unsigned char> mRxBuffer;
    volatile bool mIoThrShouldStop;
    std::thread mIoThr;
    //
    void InstallActions();
};
