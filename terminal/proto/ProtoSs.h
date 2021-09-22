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
    //
    bool SupportTransmitData() const { return true; }
    int Transmit(const void* input, size_t input_size, std::string& errmsg);
};

}
