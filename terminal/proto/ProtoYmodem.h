//
// (c) 2020 chiv
//
#include "Proto.h"

namespace proto {

class ProtoYmodem : public Proto {
public:
    ProtoYmodem(SerialConn* conn);
    ~ProtoYmodem();
    //
    std::string GetDescription() const;
    // return  0 Absolutely not
    //         1 Pending
    //        >1 Yes
    int IsProto(const unsigned char* buf, size_t sz);
    //
    int TransmitFile(const std::string& filename, std::string& errmsg);
    int TransmitFrame(const void* frm, size_t frm_size, unsigned char frm_idx, std::string& errmsg);
    //
    int Transmit(const void* input, size_t input_size, std::string& errmsg);
    // input_size <= 128, otherwise input will be truncated.
    std::vector<unsigned char> Pack(const void* input, size_t input_size, size_t pkt_sz,
        unsigned char pad, unsigned char pkt_idx = 1);

};

}
