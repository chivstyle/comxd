//
// (c) 2020 chiv
//
#include "resource.h"
#include "SerialPort.h"
#include "SerialDevsDialog.h"

SerialPort::SerialPort(std::shared_ptr<serial::Serial> serial)
    : mSerial(serial)
{
    mUsrActions.emplace_back(comxd::settings(), t_("Settings"), t_("open device settings dialog"), [=]() {
        SerialDevsDialog d;
        d.ChangeSettings(this);
    });
}
