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
    try {
        if (!mSerial->isOpen()) {
            mSerial->open();
        }
        return true;
    } catch (const std::exception& ex) {
        (void)ex;
    }
    return false;
}

void SerialPort::Stop()
{
	mSerial->close();
}

bool SerialPort::Reconnect()
{
    Stop();
    //
    SerialDevsDialog d;
	if (d.Reconnect(this)) {
	    return Start();
	}
}
