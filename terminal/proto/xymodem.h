/*!
// (c) 2021 chiv
//
*/
#pragma once

// ref:http://web.mit.edu/6.115/www/amulet/xmodem.htm
namespace xymodem {
    static const char fNUL = 0;
    static const char fDLE = 16;
    static const char fSOH = 1;
    static const char fSTX = 2;
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
    static const int kTimeout = 1000;
    static const int kSyncTimeout = 10000;
    //
    enum ChecksumType {
        CSUM,
        CRC16
    };
    // prefix
    // 128 SOH
    // 1024 STX
    static inline std::vector<unsigned char> Pack(const void* input, size_t input_size, size_t pkt_sz,
        unsigned char prefix, unsigned char pad, int cs_type, unsigned char pkt_idx)
    {
        std::vector<unsigned char> out(3+pkt_sz, fEOF);
        for (size_t k = 0; k < input_size && k < pkt_sz; ++k) {
            out[k + 3] = *((const unsigned char*)input + k);
        }
        for (size_t k = input_size; k < pkt_sz; ++k) {
            out[k + 3] = pad;
        }
        out[0] = prefix;
        out[1] = pkt_idx;
        out[2] = ~pkt_idx;
        switch (cs_type) {
        case CRC16:if (1) {
            unsigned short crc = (unsigned short)calcrc(out.data() + 3, pkt_sz);
            out.push_back((unsigned char)(crc >> 8));
            out.push_back((unsigned char)(crc & 0xff));
        } break;
        case CSUM: if (1) {
            size_t sum = 0;
            for (size_t k = 0; k < pkt_sz; ++k) {
                sum += out[k + 3];
            }
            out.push_back((unsigned char)(sum & 0xff));
        } break;
        }
        return out;
    }
    //
    static inline std::string _Filename(const std::string& pathname)
    {
        int len = (int)pathname.length();
        while (pathname[len] != '/' && pathname[len] != '\\') {
            len--;
        }
        // len >= 0, found 1 path prefix at least
        return len >= 0 ? pathname.substr(len + 1) : pathname;
    }

}
