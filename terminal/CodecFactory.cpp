//
// (c) 2020 chiv
//
#include "CodecFactory.h"

CodecFactory::CodecFactory()
{
}

CodecFactory::~CodecFactory()
{
}

CodecFactory* CodecFactory::Inst()
{
    static CodecFactory inst;
    return &inst;
}

Codec* CodecFactory::CreateInst(const char* codec_name)
{
    auto it = mInstFuncs.find(codec_name);
    if (it != mInstFuncs.end()) {
        return it->second();
    }
    return nullptr;
}
