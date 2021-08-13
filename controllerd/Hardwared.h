//
// (c) 2021 chiv
//
// Hardware of controller. A MCU, communicates with other devices
// by USART.
//
#ifndef _controllerd_Hardwared_h_
#define _controllerd_Hardwared_h_

#include <Core/Core.h>
#include <serial/serial.h>
#include <stdint.h>

class Hardwared {
public:
    // basic configuration
    // devname
    // baudrate
    // bytebits : 8
    // parity: None
    // flowcontrol: None
    // stopbits: 1
    // throws std::exception on error
    Hardwared(const std::string& devname, uint32_t baudrate);
    Hardwared(const Upp::Value& conf);
    virtual ~Hardwared();
    //
    void Run(volatile bool* should_exit);
    // command - JSON Object
    bool Send(const Upp::String& command);
    // The hardware generate some event(s), this thread will wrap
    // it in JSON, and then invoke this function.
    Upp::Event<Upp::String> WhenMessage;
    //
private:
    serial::Serial* mIo;
};

#endif
