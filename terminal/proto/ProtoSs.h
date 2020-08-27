//
// (c) 2020 chiv
//
#include "Proto.h"

namespace proto {
    
class ProtoSs : public Proto {
public:
    ProtoSs();
    virtual ~ProtoSs();
    
    virtual std::string GetName() const;
    virtual std::string GetDescription() const;
    // use this proto to pack the data
    virtual std::vector<unsigned char> Pack(const unsigned char* buf, size_t sz, std::string& errmsg);
    // return  0 Absolutely not
    //         1 Pending
    //        >1 Yes
    virtual int IsProto(const unsigned char* buf, size_t sz);
    // parse the proto message, generate report.
    virtual std::string Parse(const unsigned char* buf, size_t sz);
};
    
}
