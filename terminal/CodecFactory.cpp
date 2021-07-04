//
// (c) 2020 chiv
//
#include "CodecFactory.h"
#include "terminal_rc.h"

using namespace Upp;

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

Codec* CodecFactory::CreateInst(const String& codec_name)
{
    auto it = mInsts.find(codec_name);
    if (it != mInsts.end()) {
        return it->second();
    }
    return nullptr;
}
