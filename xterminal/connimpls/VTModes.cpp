//
// (c) 2021 chiv
//
#include "VTModes.h"

using namespace xvt;

VTModes::VTModes()
{
}

#define GET_METHOD(type) \
int VTModes::Get##type##Mode(int mode, int def) \
{ \
	auto it = m##type##Modes.find(mode); \
	if (it != m##type##Modes.end()) \
		return it->second; \
	return def; \
}
#define SET_METHOD(type) \
void VTModes::Set##type##Mode(int mode, int val) \
{ \
	m##type##Modes[mode] = val; \
}

GET_METHOD(Ansi)
GET_METHOD(Decp)
SET_METHOD(Ansi)
SET_METHOD(Decp)