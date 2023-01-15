//
// (c) 2020 chiv
//
#include "resource.h"
#include "SerialPort.h"
#include "SerialDevsDialog.h"

SerialPort::SerialPort(std::shared_ptr<serial::Serial> serial)
    : mSerial(serial)
{
}
const char* SerialPort::kDeviceType = "Serial";

bool SerialPort::Start()
{
	if (!mSerial->isOpen()) {
		SerialDevsDialog d;
		return d.Reconnect(this);
	}
	return true;
}

void SerialPort::Stop()
{
	mSerial->close();
}
