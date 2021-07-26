/*!
// (c) 2021 chiv
//
*/
#pragma once

// ref:http://web.mit.edu/6.115/www/amulet/xmodem.htm
namespace xyzmodem {
    static const char fDLE = 16;
    static const char fSOH = 1;
    static const char fEOT = 4;
    static const char fACK = 6;
    static const char fNAK = 21;
    static const char fCAN = 24;
    static const char fEOF = 26;
    static const char fETB = 23;
    //
    static inline int calcrc(const unsigned char* ptr, int count)
    {
        int crc;
        char i;
        crc = 0;
        while (--count >= 0) {
            crc = crc ^ (int)*ptr++ << 8;
            i = 8;
            do {
                if (crc & 0x8000)
                    crc = crc << 1 ^ 0x1021;
                else
                    crc = crc << 1;
            } while (--i);
        }
        return (crc);
    }
    //
    static const int64_t kMaxFileSize = 1024 * 1024 * 64;
}
