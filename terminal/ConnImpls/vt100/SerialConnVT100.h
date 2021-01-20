/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "SerialConnVT102.h"

class SerialConnVT100 : public SerialConnVT102 {
public:
	SerialConnVT100(std::shared_ptr<SerialIo> io);
protected:
	virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
};
