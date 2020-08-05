//
// (c) 2020 chiv
//
#include "SerialConnXterm.h"
#include "XtermControlSeq.h"
#include "ConnFactory.h"


REGISTER_CONN_INSTANCE("Xterm", SerialConnXterm);

using namespace Upp;

SerialConnXterm::SerialConnXterm(std::shared_ptr<serial::Serial> serial)
    : SerialConnECMA48(serial)
{
    InstallControlSeqHandlers();
}
//
SerialConnXterm::~SerialConnXterm()
{
}

int SerialConnXterm::IsControlSeq(const std::string& seq)
{
    int ret = IsXtermControlSeq(seq);
    if (ret == 0) { // It's not a xterm control seq absolutely
        return Superclass::IsControlSeq(seq);
    }
    return ret;
}

void SerialConnXterm::InstallControlSeqHandlers()
{
}
