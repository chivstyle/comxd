#ifndef _comxd_SerialConnForm_h_
#define _comxd_SerialConnForm_h_

#include "Conn.h"
// c++11 thread
#include <mutex>
#include <thread>
#include <vector>

class Proto;

class SerialConnRaw : public WithConnRaw<SerialConn> {
public:
    typedef SerialConnRaw CLASSNAME;
    // SerialConnRaw will manage the life of serial.
    SerialConnRaw(std::shared_ptr<SerialIo> io);
    virtual ~SerialConnRaw();
    //
    bool Start();
    //
    void Stop();
    //
protected:
    static const int kPeriodicTimerId = 0;
    std::thread mRxThr; //<! thread to receive data
    std::vector<unsigned char> mRxBuffer;
    volatile bool mRxShouldStop;
    volatile bool mStopUpdateRx;
    std::mutex mRxBufferLock; //<! protect the received buffer
    void RxProc();
    //
    void OnSend();
    //
    void UpdateAsTxt();
    void UpdateAsHex();
    //
    void InstallActions();
    // UI
    void Set_TxInHex();
    void Set_TxInTxt();
    //
    void Update();
    //
private:
    Upp::LineEdit mRx;
    Upp::LineEdit mTx;
    Proto* mTxProto;
    void InstallUsrActions();
};

#endif
