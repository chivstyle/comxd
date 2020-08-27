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
    virtual std::string GetName() const;
    virtual std::string GetDescription() const;
    // use this proto to pack the data
    virtual std::vector<unsigned char> Pack(const unsigned char* buf, size_t sz, std::string& errmsg) = 0;
    // return  0 Absolutely not
    //         1 Pending
    //        >1 Yes
    virtual int IsProto(const unsigned char* buf, size_t sz) = 0;
    // parse the proto message, generate report.
    virtual std::string Parse(const unsigned char* buf, size_t sz) = 0;
    //
    const std::list<UsrAction>& GetActions() const { return mUsrActions; }
    //
protected:
    std::list<UsrAction> mUsrActions;
};

#endif
