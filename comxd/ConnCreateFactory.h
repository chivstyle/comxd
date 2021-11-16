//
// (c) 2021 chiv
//
#pragma once

#include "Conn.h"
#include <map>

class ConnCreateFactory {
public:
    static ConnCreateFactory* Inst();
    //
    typedef std::function<SerialConn*()> FnCreateConnFunc;
    class ConnIntroduction {
    public:
        Upp::String Name;
        Upp::String Desc;
        Upp::Image  Icon;
        FnCreateConnFunc Create;
        ConnIntroduction(const Upp::String& name, const Upp::String& desc,
            const Upp::Image& icon, FnCreateConnFunc func)
            : Name(name)
            , Desc(desc)
            , Icon(icon)
            , Create(func)
        {
        }
    };
    //
    SerialConn* CreateConn(const Upp::String& name);
    //
    const std::map<Upp::String, ConnIntroduction>& GetSupportedConnIntroductions() const { return mConns; }
    //
    bool RegisterInstanceFunc(const Upp::String& name, const Upp::String& desc, const Upp::Image& icon,
        FnCreateConnFunc func);
    //
private:
    std::map<Upp::String, ConnIntroduction> mConns;
    //
    ConnCreateFactory() {}
};
