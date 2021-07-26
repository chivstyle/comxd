//
// (c) 2020 chiv
//
#include "ProtoXmodem.h"
#include "xyzmodem.h"
#include "Conn.h"
#include "ProtoFactory.h"
#include "terminal_rc.h"
#include <thread>

namespace proto {

REGISTER_PROTO_INSTANCE("XMODEM", ProtoXmodem);

ProtoXmodem::ProtoXmodem(SerialConn* conn)
    : Proto(conn)
{
    WhenUsrBar = [=](Bar& bar) {
        bar.Add(t_("About"), terminal::help(), [=]() { Upp::PromptOK("Standard XMODEM/CRC v1.0a"); });
        bar.Add(t_("Transmit File..."), [=]() {
            FileSel fs;
            if (fs.AllFilesType().ExecuteOpen()) {
                auto filename = fs.Get();
                std::string errmsg;
                TransmitFile(filename.ToStd(), errmsg);
                if (!errmsg.empty()) {
                    PromptOK(errmsg.c_str());
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
            if (buf[0] == xyzmodem::fSOH && xyzmodem::calcrc(buf + 1, 128) == (((int)buf[131] << 8) | buf[132]))
                return 2;
            else if (buf[0] == xyzmodem::fSOH)
                return 1;
        }
    }
    return 0;
}

std::vector<unsigned char> ProtoXmodem::Pack(const void* input, size_t input_size, size_t pkt_sz,
    unsigned char pad, unsigned char pkt_idx)
{
    std::vector<unsigned char> out(3+pkt_sz+2, xyzmodem::fEOF);
    for (size_t k = 0; k < input_size && k < pkt_sz; ++k) {
        out[k + 3] = *((const unsigned char*)input + k);
    }
    for (size_t k = input_size; k < pkt_sz; ++k) {
        out[k + 3] = pad;
    }
    out[0] = xyzmodem::fSOH;
    out[1] = pkt_idx;
    out[2] = ~pkt_idx;
    unsigned short crc = (unsigned short)xyzmodem::calcrc(out.data() + 3, pkt_sz);
    out[3+pkt_sz] = (unsigned char)(crc >> 8);
    out[4+pkt_sz] = (unsigned char)(crc & 0xff);
    //
    return out;
}
// find token from response, return the first token
static inline int expect_resp(SerialIo* io, int timeout, volatile bool* should_stop, const std::vector<char>& tks)
{
    while (timeout >= 0 && !*should_stop) {
        int sz = io->Available();
        if (sz < 0)
            break; // The IO device was corrupted
        if (sz > 0) {
            auto resp = io->ReadRaw(1);
            for (size_t k = 0; k < tks.size(); ++k) {
                auto it = std::find(resp.begin(), resp.end(), tks[k]);
                if (it != resp.end())
                    return *it;
            }
        } else
            std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
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
        if (sz < 0)
            break; // The IO device was corrupted
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
        } else
            std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
        //
        timeout -= 10;
    }
    return false;
}

int ProtoXmodem::TransmitFile(const std::string& filename, std::string& errmsg)
{
    Progress bar;
    const int kTimeout = 1500;
    auto io = mConn->GetIo();
    bool failed = false;
    volatile bool should_stop = false;
    int input_size = -1;
    //
    bar.Title(filename.c_str());
    bar.WhenClose = [&]() { should_stop = true; };
    FileIn fin;
    if (fin.Open(filename.c_str())) {
        auto filesz = fin.GetSize();
        if (filesz > xyzmodem::kMaxFileSize) {
            errmsg = filename + " is too large to transmit, max:" + std::to_string(xyzmodem::kMaxFileSize);
            return -1;
        }
        //
        size_t blkcnt = filesz / 128;
        size_t leftcn = filesz % 128;
        size_t totcnt = leftcn ? blkcnt + 1 : blkcnt;
        size_t k;
        // create the job
        auto job = [&]() {
            unsigned char idx = 1; // xyzmodem begins from 1
            // wait for sync
            if (!expect_seqs(io, kTimeout, &should_stop, { 'C' })) {
                failed = true;
                errmsg = "Sync Timeout!";
            }
            //
            char chunk[128];
            for (k = 0; k < totcnt && !should_stop && !failed; ++k) {
                auto frmsz = fin.Get(chunk, 128);
                if (TransmitFrame(chunk, frmsz, idx++, errmsg) < 0) {
                    failed = true;
                    break;
                }
                //
                PostCallback([&]() { bar.Set(k, totcnt); });
            }
            // write the tail seq
            if (!failed) {
                failed = true;
                io->Write((const unsigned char*)&xyzmodem::fEOT, 1);
                if (expect_resp(io, kTimeout, &should_stop, { xyzmodem::fACK }) == xyzmodem::fACK) {
                    //io->Write((const unsigned char*)&xyzmodem::fETB, 1);
                    //if (expect_resp(io, kTimeout, &should_stop, {xyzmodem::fACK}) == xyzmodem::fACK) {
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
    } else {
        failed = true;
        errmsg = std::string("failed to open file:") + filename;
    }
    return failed ? 0 : (int)input_size;
}

int ProtoXmodem::TransmitFrame(const void* frm, size_t frm_size, unsigned char frm_idx, std::string& errmsg)
{
    bool failed = false;
    const int kTimeout = 1500;
    volatile bool should_stop = false;
    auto io = mConn->GetIo();
    size_t blksz = std::min(frm_size, size_t(128));
    auto pkt = Pack(frm, blksz, 128, xyzmodem::fEOF, frm_idx);
    int retry = 3;
    while (retry--) {
        io->Write(pkt);
        // wait for the response
        int ret = expect_resp(io, kTimeout, &should_stop, { xyzmodem::fACK, xyzmodem::fNAK, xyzmodem::fCAN });
        if (ret < 0) {
            errmsg = "The remote device was not responsed in time";
            failed = true;
            break;
        } else if (ret == xyzmodem::fNAK) { // retransmit
            io->Write(pkt);
        } else if (ret == xyzmodem::fACK) {
            break;
        } else if (ret == xyzmodem::fCAN) {
            errmsg = "The remote device canceled the transmit";
            failed = true;
            break;
        }
    }
    return failed ? -1 : (int)blksz;
}

int ProtoXmodem::Transmit(const void* input, size_t input_size, std::string& errmsg)
{
    (void)input;
    (void)input_size;
    errmsg = "Not supported";
    return -1;
}

}
