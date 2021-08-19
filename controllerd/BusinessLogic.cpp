/*!
// (c) 2021 chiv
//
*/
#include "BusinessLogic.h"
//
#include "Hardwared.h"
#include "HardwareSpec.h"
#include "WsServerd.h"
#include "Actuator.h"
//
using namespace Upp;

class s_exception : public std::exception {
public:
	static const int kMaxTextSize = 512;
	s_exception(const char* fmt, ...)
	{
		va_list va;
		va_start(va, fmt);
		vsnprintf(mText, kMaxTextSize, fmt, va);
		va_end(va);
	}
	s_exception()
		: mText("")
	{
	}
	const char* what() const throw() { return mText; }
	
private:
	//
	char mText[kMaxTextSize];
};

BusinessLogic::BusinessLogic(const String& conf_file)
{
	mConf = ParseJSON(LoadFile(conf_file));
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
	Actuator actuator(mConf["Actuator"]);
	HardwareSpec spec (&hardware, &wsserver, &actuator);
	//
	Thread _1, _2, _3;
	_1.Run([&]() { hardware.Run(should_exit); });
	_2.Run([&]() { wsserver.Run(should_exit); });
	_3.Run([&]() { actuator.Run(should_exit); });
	//
	spec.Run(should_exit);
	//
	_1.Wait();
	_2.Wait();
}
