/*!
// (c) 2021 chiv
//
*/
#include "SerialConnAnsi.h"
#include "AnsiCharset.h"
#include "ConnFactory.h"

REGISTER_CONN_INSTANCE("ansi by chiv", "ansi", SerialConnAnsi);

SerialConnAnsi::SerialConnAnsi(std::shared_ptr<SerialIo> io)
	: SerialConnVT100(io)
	, SerialConnVT(io)
{
}

uint32_t SerialConnAnsi::RemapCharacter(uint32_t uc)
{
	if (uc >= 0x80 && uc < 0xff) {
	    return Ansi_RemapCharacter(uc);
	} else return SerialConnVT100::RemapCharacter(uc);
}
