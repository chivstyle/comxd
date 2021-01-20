/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT100.h"
#include "ConnFactory.h"

REGISTER_CONN_INSTANCE("VT100 by chiv", "vt100", SerialConnVT100);

using namespace Upp;

SerialConnVT100::SerialConnVT100(std::shared_ptr<SerialIo> io)
	: SerialConnVT102(io)
	, SerialConnVT(io)
{
}

bool SerialConnVT100::ProcessKeyDown(Upp::dword key, Upp::dword flags)
{
	bool processed = false;
	if (flags == 0) {
		processed = true;
		switch (key) {
		case K_F1:  GetIo()->Write("\x1bOP"); break;
		case K_F2:  GetIo()->Write("\x1bOQ"); break;
		case K_F3:  GetIo()->Write("\x1bOR"); break;
		case K_F4:  GetIo()->Write("\x1bOS"); break;
		case K_F5:  GetIo()->Write("\x1bOt"); break;
		case K_F6:  GetIo()->Write("\x1bOu"); break;
		case K_F7:  GetIo()->Write("\x1bOv"); break;
		case K_F8:  GetIo()->Write("\x1bOl"); break;
		case K_F9:  GetIo()->Write("\x1bOw"); break;
		case K_F10: GetIo()->Write("\x1bOx"); break;
		default: processed = false; break;
		}
	}
	return processed ? true : SerialConnVT102::ProcessKeyDown(key, flags);
}
