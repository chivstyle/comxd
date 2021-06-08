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
    static const char fETB = 23;
    //
    unsigned short calcrc(const unsigned char *ptr, int len)
    {
        unsigned int i;
        unsigned short crc = 0x0000;
        
        while (len--) {
            crc ^= (unsigned short)(*ptr++) << 8;
            for (i = 0; i < 8; ++i) {
                if (crc & 0x8000)
                    crc = (crc << 1) ^ 0x1021;
                else
                    crc <<= 1;
            }
        }
        return crc;
    }
    //
    static const int64 kMaxFileSize = 1024*1024*64;
}

ProtoXmodem::ProtoXmodem(SerialConn* conn)
    : Proto(conn)
{
    WhenUsrBar = [=](Bar& bar) {
        bar.Add(t_("About"), terminal::help(), [=]() {
            Upp::PromptOK("Standard XMODEM/CRC v1.0a");
        });
        bar.Add(t_("Transmit File..."), [=]() {
            FileSel fs;
            if (fs.AllFilesType().ExecuteOpen()) {
                auto filename = fs.Get();
                FileIn fin;
                if (fin.Open(filename)) {
                    int64 filesz = fin.GetSize();
                    if (filesz > xmodem::kMaxFileSize) {
                        PromptOK(t_("The file is too big to transmit!"));
                    } else {
                        char* buff = new char[filesz];
                        std::string errmsg;
                        fin.Get(buff, filesz);
                        this->Transmit(buff, filesz, errmsg);
                        delete[] buff;
                        if (!errmsg.empty()) {
                            PromptOK(errmsg.c_str());
                        }
                    }
                    fin.Close();
                }
            }
        });
    };
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
// find token from response, return the first token
static inline int expect_resp(SerialIo* io, int timeout, volatile bool* should_stop, const std::vector<char>& tks)
{
    std::vector<unsigned char> buff;
    while (timeout >= 0 && !*should_stop) {
        int sz = io->Available();
        if (sz < 0) break; // The IO device was corrupted
        if (sz > 0) {
            auto resp = io->ReadRaw(sz);
            buff.insert(buff.end(), resp.begin(), resp.end());
            for (size_t k = 0; k < tks.size(); ++k) {
                auto it = std::find(buff.begin(), buff.end(), tks[k]);
                if (it != buff.end())
                    return *it;
            }
        } else std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
        //
        timeout -= 10;
    }
    return -1;
}
// find sequence from response, return 1 for matched seq, others for failure
static inline bool expect_seqs(SerialIo* io, int timeout, volatile bool* should_stop, const std::vector<char>& tks)
{
    std::vector<unsigned char> buff;
    while (timeout >= 0 && !*should_stop) {
        int sz = io->Available();
        if (sz < 0) break; // The IO device was corrupted
        if (sz > 0) {
            auto resp = io->ReadRaw(sz);
            buff.insert(buff.end(), resp.begin(), resp.end());
            auto it = std::find(buff.begin(), buff.end(), tks[0]);
            size_t k = 0;
            for (; k < tks.size() && it != buff.end(); ++k, ++it) {
                if (tks[k] != *it)
                    break;
            }
            if (k == tks.size())
                return true;
        } else std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
        //
        timeout -= 10;
    }
    return false;
}

int ProtoXmodem::Transmit(const void* input, size_t input_size, std::string& errmsg)
{
    Progress bar;
    bool failed = false;
    volatile bool should_stop = false;
    const int kTimeout = 1500;
    auto io = mConn->GetIo();
    //
    bar.WhenClose = [&]() { should_stop = true; };
    //
    size_t blkcnt = input_size / 128;
    size_t leftcn = input_size % 128;
    size_t totcnt = leftcn ? blkcnt + 1 : blkcnt;
    size_t k;
    // create the job
    auto job = [&]() {
        unsigned char idx = 1; // xmodem begins from 1
        // wait for sync
        if (!expect_seqs(io, kTimeout, &should_stop, {'C'})) {
            failed = true;
            errmsg = "Sync Timeout!";
        }
        //
        char chunk[128];
        for (k = 0; k < totcnt && !should_stop && !failed; ++k) {
            memset(chunk, xmodem::fEOF, 128);
            if (k == blkcnt) {
                memcpy(chunk, (const char*)input + k*128, leftcn);
            } else {
                memcpy(chunk, (const char*)input + k*128, 128);
            }
            auto pkt = Pack(chunk, 128, idx++);
            int retry = 3;
            while (retry--) {
                io->Write(pkt);
                // wait for the response
                int ret = expect_resp(io, kTimeout, &should_stop, {xmodem::fACK, xmodem::fNAK, xmodem::fCAN});
                if (ret < 0) {
                    errmsg = "The remote device was not responsed in time";
                    failed = true;
                    break;
                } else if (ret == xmodem::fNAK) { // retransmit
                    io->Write(pkt);
                } else if (ret == xmodem::fACK) {
                    break;
                } else if (ret == xmodem::fCAN) {
                    errmsg = "The remote device canceled the transmit";
                    failed = true;
                    break;
                }
            }
            PostCallback([&]() { bar.Set(k, totcnt); });
        }
        // write the tail seq
        if (!failed) {
            failed = true;
            io->Write((const unsigned char*)&xmodem::fEOT, 1);
            if (expect_resp(io, kTimeout, &should_stop, {xmodem::fACK}) == xmodem::fACK) {
                //io->Write((const unsigned char*)&xmodem::fETB, 1);
                //if (expect_resp(io, kTimeout, &should_stop, {xmodem::fACK}) == xmodem::fACK) {
                    // completed.
                    failed = false;
                //} else {
                //    errmsg = "ETB was not responsed!";
                //}
            } else {
                errmsg = "EOT was not responsed!";
            }
        }
        Upp::PostCallback([&]() { bar.Close(); });
    };
    // before transmit, we should stop the conn
    mConn->Stop();
    auto thr = std::thread(job);
    bar.RunAppModal();
    thr.join();
    // after transmit, restart the conn
    mConn->Start();
    return failed ? 0 : (int)input_size;
}
    
}
