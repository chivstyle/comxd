//
// (c) 2020 chiv
//
#ifndef _proto_proto_factory_h
#define _proto_proto_factory_h

#include "Proto.h"
#include <map>

#ifndef DELETE_CA_FUNCTIONS
#define DELETE_CA_FUNCTIONS(class_name) class_name(const class_name&) = delete; \
    class_name(class_name&&) = delete; \
    class_name& operator=(const class_name&) = delete; \
    class_name& operator=(class_name&&) = delete
#endif

class ProtoFactory {
public:
    static ProtoFactory* Inst();
    // create instance
    // type_name - Type name of conn, such as VT102, Xterm, Modbus RTU, .etc
    Proto* CreateInst(const Upp::String& proto_name);
    //
    bool RegisterCreateInstFunc(const Upp::String& proto_name,
                                std::function<Proto*()> func)
    {
        if (mInsts.find(proto_name) == mInsts.end()) {
            mInsts[proto_name] = func;
            return true;
        } else return false; // There's already a function in the map
    }
    //
    std::vector<Upp::String> GetSupportedProtoNames() const
    {
        std::vector<Upp::String> list;
        for (auto it = mInsts.begin(); it != mInsts.end(); ++it) {
            list.push_back(it->first);
        }
        return list;
    }
    //
protected:
    // functions to create instance(s)
    std::map<Upp::String, std::function<Proto*()> > mInsts;
    
    DELETE_CA_FUNCTIONS(ProtoFactory);
    ProtoFactory();
    virtual ~ProtoFactory();
};

// Help macros
#define REGISTER_PROTO_INSTANCE(proto_name, class_name) \
class class_name##_ { \
public: \
    class_name##_() \
    { \
        ProtoFactory::Inst()->RegisterCreateInstFunc(proto_name, \
        [=]()->Proto* { \
            auto inst_ = new class_name(); \
            inst_->SetName(proto_name); \
            return inst_; \
        }); \
    } \
}; \
static class_name##_ q##class_name##_


#endif
