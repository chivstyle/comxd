//
// (c) 2020 chiv
//
#include "ProtoYmodem.h"

namespace proto {

class ProtoStandardYmodem : public ProtoYmodem {
public:
    ProtoStandardYmodem(SerialConn* conn);
};

}
