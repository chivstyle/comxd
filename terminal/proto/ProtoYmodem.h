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
    // please invoke these functions from GUI-thread
    int TransmitData(const void* input, size_t input_size, std::string& errmsg);
    int TransmitFile();
    //
protected:
    int TransmitFile(const std::string& filename, std::string& errmsg, bool last_one = false);
    int TransmitFile(SerialIo* io, const std::string& filename, std::string& errmsg, bool last_one = false);
    int TransmitData(SerialIo* io, const void* input, size_t input_size, std::string& errmsg);
};

}
