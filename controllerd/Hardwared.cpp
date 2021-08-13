//
// (c) 2021 chiv
//
#include "Hardwared.h"
#include <thread>

using namespace Upp;

Hardwared::Hardwared(const std::string& devname, uint32_t baudrate)
{
	mIo = new serial::Serial(devname, baudrate);
}

Hardwared::Hardwared(const Value& conf)
{
	mIo = new serial::Serial(conf["Name"].ToString().ToStd(),
							 conf["Baudrate"].Get<int>(),
							 serial::Timeout(),
							 static_cast<serial::bytesize_t>(conf["ByteSize"].Get<int>()),
							 static_cast<serial::parity_t>(conf["Parity"].Get<int>()),
							 static_cast<serial::stopbits_t>(conf["StopBits"].Get<int>()),
							 static_cast<serial::flowcontrol_t>(conf["FlowControl"].Get<int>()));
}

Hardwared::~Hardwared()
{
	delete mIo;
}

bool Hardwared::Send(const String& command)
{
	// TODO: to USART command
	return true;
}

void Hardwared::Run(volatile bool* should_exit)
{
	while (*should_exit == false) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
