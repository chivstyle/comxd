//
// (c) 2020 chiv
//
#include "ProtoFactory.h"

ProtoFactory::ProtoFactory()
{
}

ProtoFactory::~ProtoFactory()
{
}

ProtoFactory* ProtoFactory::Inst()
{
    static ProtoFactory inst;
    return &inst;
}

Proto* ProtoFactory::CreateInst(const Upp::String& proto_name)
{
    auto it = mInsts.find(proto_name);
    if (it != mInsts.end()) {
        return it->second();
    }
    return nullptr;
}