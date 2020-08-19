#ifndef _comxd_Conn_h_
#define _comxd_Conn_h_

#include <CtrlLib/CtrlLib.h>
#include <memory>
#include <functional>
#include <list>

#include "SerialIo.h"
#include "Action.h"


class SerialConn : public Upp::TopWindow {
public:
    explicit SerialConn(std::shared_ptr<SerialIo> serial)
        : mSerial(serial)
    {
    }
    SerialConn()
    {
    }
    //
    Upp::String ConnName() const
    {
        return mSerial->DeviceName();
    }
    //
    const std::list<UsrAction>& GetActions() const
    {
        return mUsrActions;
    }
    //
    SerialIo* GetSerial() const { return mSerial.get(); }
    
protected:
    std::shared_ptr<SerialIo> mSerial;
    std::list<UsrAction> mUsrActions;
};

#endif
