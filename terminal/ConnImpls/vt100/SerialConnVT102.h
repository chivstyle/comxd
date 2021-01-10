/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "connimpls/ecma48/SerialConnEcma48.h"

class SerialConnVT102 : public SerialConnEcma48 {
public:
	SerialConnVT102(std::shared_ptr<SerialIo> io);
	//
};
