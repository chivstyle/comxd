//
// (c) 2020 chiv
//
#ifndef _proto_h
#define _proto_h

#include "Action.h"
#include <vector>
#include <list>

class Proto {
public:
    Proto();
    virtual ~Proto();
    //
    const std::string& GetName() const { return mName; }
    void SetName(const char* name) { mName = name; }
    //
    virtual std::string GetDescription() const { return ""; }
    // use this proto to pack the data
    // A = Pack(input)
    // B = Unpack(A)
    // B == input
    virtual std::vector<unsigned char> Pack(const std::string& input, std::string& errmsg) = 0;
    // unpack the data with proto.
    virtual std::string Unpack(const std::vector<unsigned char>& msg, std::string& errmsg) = 0;
    // return  0 Absolutely not
    //         1 Pending
    //        >1 Yes
    virtual int IsProto(const unsigned char* buf, size_t sz) = 0;
    //
    const std::list<UsrAction>& GetActions() const { return mUsrActions; }
    //
protected:
    std::list<UsrAction> mUsrActions;
    std::string mName;
};

#endif
