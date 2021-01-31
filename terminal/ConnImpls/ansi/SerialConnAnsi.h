/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "connimpls/vt100/SerialConnVT100.h"

class SerialConnAnsi : public SerialConnVT100 {
public:
	SerialConnAnsi(std::shared_ptr<SerialIo> io);
	//
	void ProcessRCP(const std::string& p);
	void ProcessSCP(const std::string& p);
	// override
	uint32_t RemapCharacter(uint32_t uc, int charset);
	//
private:
	void InstallFunctions();
};
