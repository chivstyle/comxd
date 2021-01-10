/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT102.h"
#include "ConnFactory.h"

REGISTER_CONN_INSTANCE("VT102,chiv", "vt102", SerialConnVT102);

SerialConnVT102::SerialConnVT102(std::shared_ptr<SerialIo> io)
	: SerialConnVT(io)
	, SerialConnEcma48(io)
{
}