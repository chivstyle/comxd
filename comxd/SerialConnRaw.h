#ifndef _comxd_SerialConnForm_h_
#define _comxd_SerialConnForm_h_

#include "resource.h"
#include "Conn.h"
// c++11 thread
#include <thread>

class SerialConnRaw : public WithConnRaw<SerialConn> {
public:
    typedef SerialConnRaw CLASSNAME;
    // SerialConnRaw will manage the life of serial.
    SerialConnRaw(std::shared_ptr<serial::Serial> serial);
    virtual ~SerialConnRaw();
    //
protected:
    std::thread mRxThr; //<! thread to receive data
    std::vector<unsigned char> mRxBuffer;
    volatile bool mRxShouldStop;
    std::mutex mRxBufferLock; //<! protect the received buffer
    void RxProc();
    //
    void OnSend();
    //
    void UpdateAsTxt();
    void UpdateAsHex();
    //
    void InstallActions();
};


#endif
