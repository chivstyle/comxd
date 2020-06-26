#ifndef _comxd_Conn_h_
#define _comxd_Conn_h_

#include <CtrlLib/CtrlLib.h>
#include "serial/serial.h"
#include <memory>

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
    serial::Serial* GetSerial() const { return mSerial.get(); }
    //
    enum ConnTypes {
        eRaw,
        eVT102,
        eModebusRTU,
        eModebusAscii
    };
    
protected:
    std::shared_ptr<serial::Serial> mSerial;
};

#endif
