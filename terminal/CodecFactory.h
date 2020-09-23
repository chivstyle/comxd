//
// (c) 2020 chiv
//
#ifndef _terminal_CodecFactory_h_
#define _terminal_CodecFactory_h_

#include "Codec.h"
#include <map>

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
    Codec* CreateInst(const char* codec_name);
    //
    bool RegisterCreateInstFunc(const char* codec_name,
                                std::function<Codec*()> func)
    {
        if (mInstFuncs.find(codec_name) == mInstFuncs.end()) {
            mInstFuncs[codec_name] = func;
            return true;
        } else return false; // There's already a function in the map
    }
    //
    std::vector<std::string> GetSupportedCodecs() const
    {
        std::vector<std::string> list;
        for (auto it = mInstFuncs.begin(); it != mInstFuncs.end(); ++it) {
            list.push_back(it->first);
        }
        return list;
    }
    //
protected:
    // functions to create instance(s)
    std::map<std::string, std::function<Codec*()> > mInstFuncs;
    
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
