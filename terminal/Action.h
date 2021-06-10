#ifndef _comxd_Action_h_
#define _comxd_Action_h_

#include <CtrlLib/CtrlLib.h>
#include <functional>

class UsrAction {
public:
    UsrAction() {}
    UsrAction(const Upp::Image& icon, const Upp::String& text,
           const Upp::String& help, std::function<void(UsrAction&)> func)
        : Icon(icon)
        , Text(text)
        , Help(help)
        , Func(func)
        , Checked(false)
        , Data(nullptr)
    {
    }
    Upp::Image Icon;                   // The icon to display
    Upp::String Text;                  // The caption
    Upp::String Help;                   // The tip
    std::function<void(UsrAction&)> Func;         // The action
    bool Checked;
    void* Data;
};

#endif
