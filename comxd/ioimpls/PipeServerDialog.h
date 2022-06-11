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
class PipeServerDialog : public WithPipeServer<TopWindow> {
public:
    typedef PipeServerDialog CLASSNAME;

    PipeServerDialog();
    // create a serialconn from current settings.
    SerialConn* RequestConn();
    //
protected:
    bool Key(Upp::dword key, int count) override;
    //
private:
    SerialConn* mConn;
    void CreateConn();
};

#endif
