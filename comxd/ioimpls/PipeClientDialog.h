//
// (c) 2022 chiv
//
#pragma once

#ifdef _WIN32

#include "Conn.h"
// device
#include <memory>
#include <queue>
//
class PipeClientDialog : public WithPipeServer<TopWindow> {
public:
    typedef PipeClientDialog CLASSNAME;

    PipeClientDialog();
    // create a serialconn from current settings.
    SerialConn* RequestConn();
    //
protected:
    bool Key(Upp::dword key, int count);
    //
private:
    SerialConn* mConn;
    void CreateConn();
};

#endif
