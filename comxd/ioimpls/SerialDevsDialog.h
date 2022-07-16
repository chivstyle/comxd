//
// (c) 2020 chiv
//
#pragma once

#include "Conn.h"
// device
#include "SerialPort.h"
#include <memory>
//
class SerialDevsDialog : public WithSerialDevs<TopWindow> {
public:
    typedef SerialDevsDialog CLASSNAME;

    SerialDevsDialog();
    // create a serialconn from current settings.
    SerialConn* RequestConn();
    //
    bool Reconnect(SerialPort* port);
    //
protected:
    bool Key(Upp::dword key, int count) override;
    //
private:
    SerialConn* mConn;
    void CreateConn();
};
