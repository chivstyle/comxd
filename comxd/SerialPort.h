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
        size_t sz = 0;
        try {
            sz = mSerial->available();
        } catch (...) {}
        return sz;
    }
    //
    size_t Read(unsigned char* buf, size_t sz)
    {
        size_t sz_ = 0;
        try {
            sz_ = mSerial->read(buf, sz);
        } catch (...) {}
        return sz_;
    }
    //
    size_t Write(const unsigned char* buf, size_t sz)
    {
        size_t sz_ = 0;
        try {
            sz_ = mSerial->write(buf, sz);
        } catch (...) {}
        return sz_;
    }
    //
    std::string DeviceName() const
    {
        std::string name;
        try {
            name = mSerial->getPort();
        } catch (...) {}
        return name;
    }
    //
    serial::Serial* nativeDevice() const { return mSerial.get(); }
    //
private:
    std::shared_ptr<serial::Serial> mSerial;
};

#endif
