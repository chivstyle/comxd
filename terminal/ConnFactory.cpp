//
// (c) 2020 chiv
//
#include "ConnFactory.h"
#include "terminal_rc.h"

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

std::vector<String> ConnFactory::GetSupportedConnNames() const
{
    std::vector<String> list;
    for (auto it = mInsts.begin(); it != mInsts.end(); ++it) {
        list.push_back(it->first);
    }
    return list;
}

String ConnFactory::GetConnType(const String& name) const
{
    auto it = mInsts.find(name);
    if (it != mInsts.end()) {
        return it->second.first;
    }
    return "";
}

ConnFactory::CreateInstFunc ConnFactory::GetConnInstFunc(const String& name) const
{
    auto it = mInsts.find(name);
    if (it != mInsts.end()) {
        return it->second.second;
    }
    return nullptr;
}

bool ConnFactory::RegisterCreateInstFunc(const String& name, const String& type,
    std::function<SerialConn*(std::shared_ptr<SerialIo>)> func)
{
    if (mInsts.find(name) == mInsts.end()) {
        mInsts[name] = std::make_pair(type, func);
        return true;
    } else
        return false; // There's already a function in the map
}

SerialConn* ConnFactory::CreateInst(const String& name, std::shared_ptr<SerialIo> serial)
{
    auto it = mInsts.find(name);
    if (it != mInsts.end()) {
        return it->second.second(serial);
    }
    return nullptr;
}
