/*!
// (c) 2021 chiv
//
*/
#include "BusinessLogic.h"
//
#include "Hardwared.h"
#include "HardwareSpec.h"
#include "WsServerd.h"
#include "Database.h"
#include "s_exception.h"
//
using namespace Upp;

BusinessLogic::BusinessLogic(const String& conf_file)
{
#ifdef _DEBUG
	mConf = ParseJSON(LoadFile(GetDataFile(conf_file)));
#else
	mConf = ParseJSON(LoadFile(conf_file));
#endif
	if (mConf.IsNull()) {
		// use default instead
		throw s_exception("You should pass me a valid JSON configuration file");
	}
}
BusinessLogic::~BusinessLogic()
{
}

void BusinessLogic::Run(volatile bool* should_exit)
{
	Hardwared hardware(mConf["Hardwared"]);
	WsServerd wsserver(mConf["WsServerd"]);
	Database database(mConf["Database"]);
	HardwareSpec spec (&hardware, &wsserver, &database);
	//
	Thread _1, _2;
	_1.Run([&]() { hardware.Run(should_exit); });
	_2.Run([&]() { wsserver.Run(should_exit); });
	//
	spec.Run(should_exit);
	//
	_1.Wait();
	_2.Wait();
}
