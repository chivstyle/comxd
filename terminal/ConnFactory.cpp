//
// (c) 2020 chiv
//
#include "ConnFactory.h"
#include "terminal_rc.h"
#include "ConnImpls/SerialConnRaw.h"

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

std::vector<std::string> ConnFactory::GetSupportedConnTypes() const
{
    std::vector<std::string> list;
    for (auto it = mInstFuncs.begin(); it != mInstFuncs.end(); ++it) {
        list.push_back(it->first);
    }
    return list;
}

bool ConnFactory::RegisterCreateInstFunc(const char* type_name,
                            std::function<SerialConn*(std::shared_ptr<SerialIo>)> func)
{
    if (mInstFuncs.find(type_name) == mInstFuncs.end()) {
        mInstFuncs[type_name] = func;
        return true;
    } else return false; // There's already a function in the map
}

SerialConn* ConnFactory::CreateInst(const char* type_name, std::shared_ptr<SerialIo> serial)
{
    auto it = mInstFuncs.find(type_name);
    if (it != mInstFuncs.end()) {
        return it->second(serial);
    }
    return nullptr;
}
