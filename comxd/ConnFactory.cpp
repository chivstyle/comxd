//
// (c) 2020 chiv
//
#include "ConnFactory.h"
// kinds of implements
#include "SerialConnRaw.h"

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

SerialConn* ConnFactory::CreateInst(std::shared_ptr<serial::Serial> serial, int type)
{
    switch (type) {
    case SerialConn::eRaw:
        return new SerialConnRaw(serial);
    default:break;
    }
    return nullptr;
}
