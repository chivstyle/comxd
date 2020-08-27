//
// (c) 2020 chiv, all rights reserved
//

#ifndef _comxd_ConnFactory_h_
#define _comxd_ConnFactory_h_

#include "Conn.h"
#include <map>

#ifndef DELETE_CA_FUNCTIONS
#define DELETE_CA_FUNCTIONS(class_name) class_name(const class_name&) = delete; \
    class_name(class_name&&) = delete; \
    class_name& operator=(const class_name&) = delete; \
    class_name& operator=(class_name&&) = delete
#endif

class ConnFactory {
public:
    static ConnFactory* Inst();
    // create instance
    // type_name - Type name of conn, such as VT102, Xterm, Modbus RTU, .etc
    SerialConn* CreateInst(const char* type_name, std::shared_ptr<SerialIo> serial);
    //
    bool RegisterCreateInstFunc(const char* type_name,
                                std::function<SerialConn*(std::shared_ptr<SerialIo>)> func);
    //
    std::vector<std::string> GetSupportedConnTypes() const;
    //
protected:
    // functions to create instance(s)
    std::map<std::string,
        std::function<SerialConn*(std::shared_ptr<SerialIo>)> > mInstFuncs;
    
    DELETE_CA_FUNCTIONS(ConnFactory);
    ConnFactory();
    virtual ~ConnFactory();
};

// Help macros
#define REGISTER_CONN_INSTANCE(type_name, class_name) \
class class_name##_ { \
public: \
    class_name##_() \
    { \
        ConnFactory::Inst()->RegisterCreateInstFunc(type_name, \
        [=](std::shared_ptr<SerialIo> serial)->SerialConn* { \
            return new class_name(serial); \
        }); \
    } \
}; \
static class_name##_ q##class_name##_

#endif
