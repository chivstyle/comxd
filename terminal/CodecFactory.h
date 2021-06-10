//
// (c) 2020 chiv
//
#ifndef _terminal_CodecFactory_h_
#define _terminal_CodecFactory_h_

#include "Codec.h"
#include <CtrlLib/CtrlLib.h>
#include <map>
#include <functional>

#ifndef DELETE_CA_FUNCTIONS
#define DELETE_CA_FUNCTIONS(class_name) class_name(const class_name&) = delete; \
    class_name(class_name&&) = delete; \
    class_name& operator=(const class_name&) = delete; \
    class_name& operator=(class_name&&) = delete
#endif

class CodecFactory {
public:
    static CodecFactory* Inst();
    // create instance
    // type_name - Type name of conn, such as VT102, Xterm, Modbus RTU, .etc
    Codec* CreateInst(const Upp::String& codec_name);
    //
    bool RegisterCreateInstFunc(const Upp::String& codec_name,
                                std::function<Codec*()> func)
    {
        if (mInsts.find(codec_name) == mInsts.end()) {
            mInsts[codec_name] = func;
            return true;
        } else return false; // There's already a function in the map
    }
    //
    std::vector<Upp::String> GetSupportedCodecNames() const
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
    std::map<Upp::String, std::function<Codec*()> > mInsts;
    
    DELETE_CA_FUNCTIONS(CodecFactory);
    CodecFactory();
    virtual ~CodecFactory();
};

// Help macros
#define REGISTER_CODEC_INSTANCE(codec_name, class_name) \
class class_name##_ { \
public: \
    class_name##_() \
    { \
        CodecFactory::Inst()->RegisterCreateInstFunc(codec_name, \
        [=]()->Codec* { \
            auto inst_ = new class_name(); \
            inst_->SetName(codec_name); \
            return inst_; \
        }); \
    } \
}; \
static class_name##_ q##class_name##_


#endif
