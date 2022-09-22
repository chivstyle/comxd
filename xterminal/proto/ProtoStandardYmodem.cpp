//
// (c) 2020 chiv
//
#include "ProtoStandardYmodem.h"
#include "ProtoFactory.h"

namespace proto {
REGISTER_PROTO_INSTANCE("YMODEM", ProtoStandardYmodem);

ProtoStandardYmodem::ProtoStandardYmodem(SerialConn* conn)
    : ProtoYmodem(conn)
{
    mType = eStandardYmodem;
}

}
