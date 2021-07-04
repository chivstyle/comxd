//
// (c) 2020 chiv
//
#ifndef _proto_h
#define _proto_h

#include <CtrlLib/CtrlLib.h>
#include <list>
#include <vector>

class SerialConn;
class Proto {
public:
    Proto(SerialConn*);
    virtual ~Proto();
    //
    const std::string& GetName() const { return mName; }
    void SetName(const char* name) { mName = name; }
    //
    virtual std::string GetDescription() const { return ""; }
    // return  0 Absolutely not
    //         1 Pending
    //        >1 Yes
    virtual int IsProto(const unsigned char* buf, size_t sz) = 0;
    // return <= 0 failed
    //        > 0  bytes transmitted
    virtual int Transmit(const void* input, size_t input_size, std::string& errmsg) = 0;
    //
    Upp::Event<Upp::Bar&> WhenUsrBar;
    //
protected:
    std::string mName;
    SerialConn* mConn;
};

#endif
