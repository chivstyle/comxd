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
    //
    bool SupportTransmitData() const { return true; }
    int Transmit(const void* input, size_t input_size, std::string& errmsg);
};

}
