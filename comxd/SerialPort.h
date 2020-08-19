#ifndef _comxd_SerialPort_h_
#define _comxd_SerialPort_h_

#include "SerialIo.h"
#include "serial/serial.h"

class SerialPort : public SerialIo {
public:
    SerialPort(std::shared_ptr<serial::Serial> serial);
    //
    size_t Available() const
    {
        return mSerial->available();
    }
    //
    size_t Read(unsigned char* buf, size_t sz)
    {
        return mSerial->read(buf, sz);
    }
    //
    size_t Write(const unsigned char* buf, size_t sz)
    {
        return mSerial->write(buf, sz);
    }
    //
    std::string DeviceName() const
    {
        return mSerial->getPort();
    }
    //
    serial::Serial* nativeDevice() const { return mSerial.get(); }
    //
private:
    std::shared_ptr<serial::Serial> mSerial;
};

#endif
