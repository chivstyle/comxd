//
// (c) 2020 chiv
//
#pragma once

#include "Conn.h"
// device
#include <memory>
//
class SSHDevsDialog : public WithSSHDevs<TopWindow> {
public:
    typedef SSHDevsDialog CLASSNAME;

    SSHDevsDialog();
    // create a serialconn from current settings.
    SerialConn* RequestConn();
    //
protected:
    bool Key(Upp::dword key, int count);
};
