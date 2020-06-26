//
// (c) 2020 chiv, all rights reserved
//

#ifndef _comxd_ConnFactory_h_
#define _comxd_ConnFactory_h_

#include "Conn.h"

#define DELETE_CA_FUNCTIONS(class_name) class_name(const class_name&) = delete; \
    class_name(class_name&&) = delete; \
    class_name& operator=(const class_name&) = delete; \
    class_name& operator=(class_name&&) = delete

class ConnFactory {
public:
    static ConnFactory* Inst();
    //
    SerialConn* CreateInst(std::shared_ptr<serial::Serial> serial, int type);
    //
protected:
    DELETE_CA_FUNCTIONS(ConnFactory);
    ConnFactory();
    virtual ~ConnFactory();
};

#endif
