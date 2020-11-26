//
// (c) 2020 chiv
//
#pragma once

#include "SerialIo.h"
#include "serial/serial.h"
#include <memory>

class SerialPort : public SerialIo {
public:
    SerialPort(std::shared_ptr<serial::Serial> serial);
    //
    int Available() const
    {
        int sz = -1;
        try {
            sz = (int)mSerial->available();
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
    serial::Serial* GetNativeDevice() const { return mSerial.get(); }
    //
private:
    std::shared_ptr<serial::Serial> mSerial;
};
