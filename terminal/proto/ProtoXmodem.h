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
    bool SupportTransmitFile() const { return true; }
    // please invoke these functions from GUI-thread
    int TransmitData(const void* input, size_t input_size, std::string& errmsg);
    int TransmitFile();
    //
protected:
    int TransmitFile(const std::string& filename, std::string& errmsg);
    int TransmitFile(SerialIo* io, const std::string& filename, std::string& errmsg);
    int TransmitData(SerialIo* io, const void* input, size_t input_size, std::string& errmsg);
    // whether to use XMODEM-k
    bool mXmodemK;
};

}
