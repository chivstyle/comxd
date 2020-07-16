//
// (c) 2020 chiv
//
#include "ConnFactory.h"
// kinds of implements
#include "SerialConnRaw.h"
#include "SerialConnVT102.h"

ConnFactory::ConnFactory()
{
}
ConnFactory::~ConnFactory()
{
}

ConnFactory* ConnFactory::Inst()
{
    static ConnFactory inst;
    return &inst;
}

SerialConn* ConnFactory::CreateInst(const char* type_name, std::shared_ptr<serial::Serial> serial)
{
    auto it = mInstFuncs.find(type_name);
    if (it != mInstFuncs.end()) {
        return it->second(serial);
    }
    return nullptr;
}
