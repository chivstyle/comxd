//
// (c) 2021 chiv
//
#include "resource.h"
#include "ConnCreateFactory.h"

ConnCreateFactory* ConnCreateFactory::Inst()
{
    static ConnCreateFactory inst;
    return &inst;
}

SerialConn* ConnCreateFactory::CreateConn(const Upp::String& name)
{
    auto it = mConns.find(name);
    if (it == mConns.end())
        return nullptr;
    return it->second.Create();
}
//
bool ConnCreateFactory::RegisterInstanceFunc(const Upp::String& name, const Upp::String& desc, const Upp::Image& icon,
    FnCreateConnFunc func)
{
    auto it = mConns.find(name);
    if (it == mConns.end()) {
        mConns.emplace(name, ConnIntroduction(name, desc, icon, func));
        return true;
    }
    return false;
}
