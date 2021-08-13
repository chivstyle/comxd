//
// (c) 2021 chiv
//
#ifndef _controllerd_BusinessLogic_h_
#define _controllerd_BusinessLogic_h_

#include "Core/Core.h"

class BusinessLogic {
public:
	// conf - A JSON file
	BusinessLogic(const Upp::String& conf_file);
	virtual ~BusinessLogic();
	//
	void Run(volatile bool* should_exit);
	//
private:
	Upp::Value mConf;
};

#endif
