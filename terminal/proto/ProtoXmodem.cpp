//
// (c) 2020 chiv
//
#include "terminal_rc.h"
#include "ProtoXmodem.h"
#include "ProtoFactory.h"
#include "Conn.h"
#include <thread>

namespace proto {
       
REGISTER_PROTO_INSTANCE("XMODEM", ProtoXmodem);
// ref:http://web.mit.edu/6.115/www/amulet/xmodem.htm
namespace xmodem {
    static const char fDLE = 16;
    static const char fSOH = 1;
    static const char fEOT = 4;
    static const char fACK = 6;
    static const char fNAK = 21;
    static const char fCAN = 24;
    static const char fEOF = 26;
    //
    int calcrc(const unsigned char *ptr, int count)
    {
        int  crc;
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
            } while(--i);
        }
        return (crc);
    }
}

ProtoXmodem::ProtoXmodem(SerialConn* conn)
    : Proto(conn)
{
}

ProtoXmodem::~ProtoXmodem()
{
}

std::string ProtoXmodem::GetDescription() const
{
    return t_("Standard XMODEM/CRC");
}
// return  0 Absolutely not
//         1 Pending
//        >1 Yes
int ProtoXmodem::IsProto(const unsigned char* buf, size_t sz)
{
    if (sz > 0) {
        if (sz >= 133) {
            if (buf[0] == xmodem::fSOH &&
                xmodem::calcrc(buf + 1, 128) == (((int)buf[131] << 8) | buf[132])) return 2;
            else if (buf[0] == xmodem::fSOH)
                return 1;
        }
    }
    return 0;
}

std::vector<unsigned char> ProtoXmodem::Pack(const void* input, size_t input_size, unsigned char pkt_idx)
{
    std::vector<unsigned char> out(133, xmodem::fEOF);
    for (size_t k = 0; k < input_size && k < 128; ++k) {
        out[k+3] = *((const unsigned char*)input + k);
    }
    out[0] = xmodem::fSOH;
    out[1] = pkt_idx;
    out[2] = ~pkt_idx;
    unsigned short crc = (unsigned short)xmodem::calcrc(out.data() + 3, 128);
    out[131] = (unsigned char)(crc >> 16);
    out[132] = (unsigned char)(crc & 0xff);
    //
    return out;
}

int ProtoXmodem::Transmit(const void* input, size_t input_size, std::string& errmsg)
{
    Progress bar;
    auto io = mConn->GetIo();
    // create the job
    [=, &io]() {
        
        size_t blkcnt = input_size / 128;
        size_t leftcn = input_size % 128;
        size_t totcnt = leftcn ? blkcnt + 1 : blkcnt;
        
        unsigned char idx = 1; // xmodem begins from 1
        
        for (size_t k = 0; k < blkcnt; ++k) {
            
            io->Write(Pack((const unsigned char*)input + 128*k, 128, idx++));
            // wait for the response
            int times = 10;
            while (times--) {
                int sz = io->Available();
                if (sz > 0) {
                    auto resp = io->ReadRaw(sz);
                    auto it = std::find(resp.begin(), resp.end(), xmodem::fACK);
                    if (it != resp.end())
                        break;
                } else std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
            }
            if (times < 0) {
                errmsg = "The remote device was not responsed in time!";
                return -1;
            }
            Upp::GuiLock __;
            bar.SetPos(k, totcnt);
        }
    };
    
    return 0;
}
    
}
