#ifndef _comxd_Conn_h_
#define _comxd_Conn_h_

#include <CtrlLib/CtrlLib.h>
#include <memory>
#include <functional>
#include <list>

#include "serial/serial.h"
#include "Action.h"


class SerialConn : public Upp::TopWindow {
public:
    explicit SerialConn(std::shared_ptr<serial::Serial> serial)
        : mSerial(serial)
    {
    }
    SerialConn()
    {
    }
    //
    Upp::String ConnName() const
    {
        return mSerial->getPort().c_str();
    }
    //
    const std::list<UsrAction>& GetActions() const
    {
        return mUsrActions;
    }
    //
    serial::Serial* GetSerial() const { return mSerial.get(); }
    
protected:
    std::shared_ptr<serial::Serial> mSerial;
    std::list<UsrAction> mUsrActions;
};

#endif
