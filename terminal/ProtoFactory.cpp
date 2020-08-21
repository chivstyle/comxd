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

Proto* ProtoFactory::CreateInst(const char* proto_name)
{
    auto it = mInstFuncs.find(proto_name);
    if (it != mInstFuncs.end()) {
        return it->second();
    }
    return nullptr;
}
