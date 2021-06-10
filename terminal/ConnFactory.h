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
    SerialConn* CreateInst(const Upp::String& name, std::shared_ptr<SerialIo> serial);
    //
    using CreateInstFunc = std::function<SerialConn*(std::shared_ptr<SerialIo>)>;
    bool RegisterCreateInstFunc(const Upp::String& name, const Upp::String& type,
                                CreateInstFunc func);
    //
    std::vector<Upp::String> GetSupportedConnNames() const;
    Upp::String GetConnType(const Upp::String& name) const;
    CreateInstFunc GetConnInstFunc(const Upp::String& name) const;
    //
protected:
    // functions to create instance(s)
    std::map<Upp::String, std::pair<Upp::String, CreateInstFunc> > mInsts;
    
    DELETE_CA_FUNCTIONS(ConnFactory);
    ConnFactory();
    virtual ~ConnFactory();
};

// Help macros
#define REGISTER_CONN_INSTANCE(name, type, class_name) \
class class_name##_ { \
public: \
    class_name##_() \
    { \
        ConnFactory::Inst()->RegisterCreateInstFunc(name, type, \
        [=](std::shared_ptr<SerialIo> serial)->SerialConn* { \
            return new class_name(serial); \
        }); \
    } \
}; \
static class_name##_ q##class_name##_

#endif
