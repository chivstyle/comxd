//
// (c) 2020 chiv
//
#include "resource.h"
#include "SerialPort.h"
#include "SerialDevsDialog.h"

SerialPort::SerialPort(std::shared_ptr<serial::Serial> serial)
    : mSerial(serial)
{
    WhenUsrBar = [=](Bar& bar) {
        bar.Add(t_("Settings"), comxd::settings(), [=]() {
            SerialDevsDialog d;
            d.ChangeSettings(this);
        }).Help("open device settings dialog");
    };
}
