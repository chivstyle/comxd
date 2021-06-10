//
// (c) 2020 chiv
//
#include "Proto.h"

namespace proto {
    
class ProtoXmodem : public Proto {
public:
    ProtoXmodem(SerialConn* conn);
    ~ProtoXmodem();
    //
    std::string GetDescription() const;
    // return  0 Absolutely not
    //         1 Pending
    //        >1 Yes
    int IsProto(const unsigned char* buf, size_t sz);
    //
    int Transmit(const void* input, size_t input_size, std::string& errmsg);
    // input_size <= 128, otherwise input will be truncated.
    std::vector<unsigned char> Pack(const void* input, size_t input_size, unsigned char pkt_idx = 1);
    
private:
    std::string mName;
};
    
}
