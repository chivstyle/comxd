//
// (c) 2020 chiv
//
#include "Proto.h"

namespace proto {

class ProtoSs : public Proto {
public:
    ProtoSs(SerialConn* conn);
    ~ProtoSs();
    //
    std::string GetDescription() const;
    // use this proto to pack the data
    std::vector<unsigned char> Pack(const void* input, size_t input_size, std::string& errmsg);
    std::string Unpack(const std::vector<unsigned char>& ss, std::string& errmsg);
    // return  0 Absolutely not
    //         1 Pending
    //        >1 Yes
    int IsProto(const unsigned char* buf, size_t sz);
    //
    int Transmit(const void* input, size_t input_size, std::string& errmsg);
};

}
