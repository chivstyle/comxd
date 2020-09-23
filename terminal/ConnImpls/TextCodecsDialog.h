//
// (c) 2020 chiv
//
#pragma once

#include <CtrlLib/CtrlLib.h>

class TextCodecsDialog : public WithTextCodecs<Upp::TopWindow> {
public:
    typedef TextCodecsDialog CLASSNAME;
    TextCodecsDialog(const char* name);
    virtual ~TextCodecsDialog();
    // Set
    Upp::String GetCodecName() const;
};
