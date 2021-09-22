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
#include <mutex>
#include <condition_variable>

class Hardwared {
public:
    struct Frame {
        unsigned char Head[2];
        unsigned char Type;
        unsigned char Order;
        unsigned char Data[64];
        unsigned char CS; // checksum, CS is standard, see XMODEM spec for detail.
        unsigned char Tail; // fixed to 0xfe
    };
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
    // Make frame that MCU supported
    // sz should be 64, if > 64, truncated, else if < 64, padding with 0.
    static std::vector<unsigned char> MakeFrame(unsigned char type, unsigned char order,
                                                const unsigned char* data, size_t sz);
    static std::vector<unsigned char> MakeFrame(unsigned char type, unsigned char order,
                                                const std::vector<unsigned char>& data);
    /// send frame to device then wait for the response.
    /// \param frame
    /// \param response
    /// \param timeout In milliseconds
    /// return
    /// - true, succeeded
    /// - false, timeout
    bool SendFrame(const std::vector<unsigned char>& frame, std::vector<unsigned char>& response,
                   int timeout, volatile bool* should_exit = nullptr);
    //
protected:
    bool IsValidFrame(const std::vector<unsigned char>& frame);
private:
    serial::Serial* mIo;
    size_t mResponseCount;
    std::vector<unsigned char> mResponse;
    std::mutex mIoLock;
    std::mutex mLock;
    std::condition_variable mCond;
};

#endif
