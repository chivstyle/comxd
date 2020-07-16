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
    virtual std::list<const UsrAction*> GetActions() const
    {
        return std::list<const UsrAction*>(); // default: No actions supported
    }
    //
    serial::Serial* GetSerial() const { return mSerial.get(); }
    
protected:
    std::shared_ptr<serial::Serial> mSerial;
};

#endif
