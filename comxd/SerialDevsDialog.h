#ifndef _comxd_SerialDevsDialog_h_
#define _comxd_SerialDevsDialog_h_

#include "resource.h"
#include "Conn.h"
#include <memory>

class SerialDevsDialog : public WithSerialDevs<TopWindow> {
public:
    typedef SerialDevsDialog CLASSNAME;
    
    SerialDevsDialog();
    // create a serialconn from current settings.
    SerialConn* RequestConn();
    //
    void ChangeSettings(serial::Serial* serial);
    //
protected:
    // open serial from current settings.
    std::shared_ptr<serial::Serial> NewSerial();
};

#endif
