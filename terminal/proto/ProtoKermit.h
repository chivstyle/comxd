//
// (c) 2020 chiv
//
#include "Proto.h"

namespace proto {

class ProtoKermit : public Proto {
public:
    ProtoKermit(SerialConn* conn);
    //
    std::string GetDescription() const;
    bool SupportTransmitFile() const { return true; }
    // please invoke these functions from GUI-thread
    int TransmitFile();
    //
protected:
    int TransmitFile(SerialIo* io, const std::string& filename, std::string& errmsg, bool last_file = false);
};

}
