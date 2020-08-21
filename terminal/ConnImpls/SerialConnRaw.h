#ifndef _comxd_SerialConnForm_h_
#define _comxd_SerialConnForm_h_

#include "Conn.h"
#include "Proto.h"
// c++11 thread
#include <thread>
#include <vector>

class SerialConnRaw : public WithConnRaw<SerialConn> {
public:
    typedef SerialConnRaw CLASSNAME;
    // SerialConnRaw will manage the life of serial.
    SerialConnRaw(std::shared_ptr<SerialIo> serial);
    virtual ~SerialConnRaw();
    //
protected:
    static const int kPeriodicTimerId = 0;
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
    //
private:
    Proto* mTxProto;
};


#endif
