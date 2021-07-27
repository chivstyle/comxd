//
// (c) 2020 chiv
//
#include "terminal_rc.h"
#include "ProtoXmodem.h"
#include "TransmitProgressDialog.h"
#include "xyzmodem.h"
#include "Conn.h"
#include "ProtoFactory.h"
#include <chrono>
#include <thread>

namespace proto {

REGISTER_PROTO_INSTANCE("XMODEM", ProtoXmodem);

ProtoXmodem::ProtoXmodem(SerialConn* conn)
    : Proto(conn)
{
    WhenUsrBar = [=](Bar& bar) {
        bar.Add(t_("About"), terminal::help(), [=]() { Upp::PromptOK("Standard XMODEM/CRC v1.0a"); });
        bar.Add(t_("Transmit File..."), [=]() {
            TransmitFile();
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
//
static inline int _TransmitFrame128(SerialIo* io, const void* frm, size_t frm_size, int cs_type, unsigned char frm_idx,
    volatile bool* should_stop, std::string& errmsg)
{
    bool failed = false;
    size_t blksz = std::min(frm_size, size_t(128));
    auto pkt = xyzmodem::Pack(frm, blksz, 128, xyzmodem::fSOH, xyzmodem::fEOF, cs_type, frm_idx);
    int retry = 3;
    while (retry--) {
        io->Write(pkt);
        // wait for the response
        int ret = expect_resp(io, xyzmodem::kTimeout, should_stop, { xyzmodem::fACK, xyzmodem::fNAK, xyzmodem::fCAN });
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
static inline int _TransmitFrame1024(SerialIo* io, const void* frm, size_t frm_size, int cs_type, unsigned char frm_idx,
    volatile bool* should_stop, std::string& errmsg)
{
    bool failed = false;
    size_t blksz = std::min(frm_size, size_t(1024));
    auto pkt = xyzmodem::Pack(frm, blksz, 1024, xyzmodem::fSTX, xyzmodem::fEOF, cs_type, frm_idx);
    int retry = 3;
    while (retry--) {
        io->Write(pkt);
        // wait for the response
        int ret = expect_resp(io, xyzmodem::kTimeout, should_stop, { xyzmodem::fACK, xyzmodem::fNAK, xyzmodem::fCAN });
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

int ProtoXmodem::TransmitFile(SerialIo* io, const std::string& filename, std::string& errmsg)
{
    TransmitProgressDialog bar;
    bool failed = false;
    static volatile bool should_stop = false;
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
        bar.SetTotal(filesz);
        //
        double ts = 0;
        size_t count = 0;
        // create the job
        auto job = [&]() {
            unsigned char idx = 1; // xyzmodem begins from 1
            // wait for sync
            int cs_type = xyzmodem::CRC16;
            int resp = expect_resp(io, xyzmodem::kTimeout, &should_stop, { 'C', xyzmodem::fNAK, 'W' });
            switch (resp) {
            case 'C': cs_type = xyzmodem::CRC16; break;
            case 'W': failed = true; errmsg = "WX-Modem CRC was not supported!"; break;
            case xyzmodem::fNAK: cs_type = xyzmodem::CSUM; break;
            case -1: failed = true; errmsg = "Unrecognized sync character!"; break;
            default: failed = true; errmsg = "Sync Timeout!"; break;
            }
            //
            auto t1 = std::chrono::high_resolution_clock::now();
            char chunk[128];
            while (!should_stop && !failed && !fin.IsEof()) {
                auto frmsz = fin.Get(chunk, 128);
                if (_TransmitFrame128(io, chunk, frmsz, cs_type, idx++, &should_stop, errmsg) < 0) {
                    failed = true;
                    break;
                }
                count += frmsz;
                //
                ts = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t1).count();
                PostCallback([&]() { bar.Update(count, count / ts); });
            }
            // write the tail seq
            if (!failed) {
                failed = true;
                io->Write((const unsigned char*)&xyzmodem::fEOT, 1);
                if (expect_resp(io, xyzmodem::kTimeout, &should_stop, { xyzmodem::fACK }) == xyzmodem::fACK) {
                    //io->Write((const unsigned char*)&xyzmodem::fETB, 1);
                    //if (expect_resp(io, xyzmodem::kTimeout, &should_stop, {xyzmodem::fACK}) == xyzmodem::fACK) {
                    // completed.
                    failed = false;
                    //} else {
                    //    errmsg = "ETB was not responsed!";
                    //}
                } else {
                    errmsg = "EOT was not responsed!";
                }
            }
            if (should_stop == true) io->Write(std::vector<unsigned char>({xyzmodem::fCAN}));
            Upp::PostCallback([&]() { bar.Close(); });
        };
        auto thr = std::thread(job);
        bar.RunAppModal();
        thr.join();
    } else {
        failed = true;
        errmsg = std::string("failed to open file:") + filename;
    }
    return failed ? T_FAILED : (int)input_size;
}

int ProtoXmodem::TransmitData(SerialIo*, const void*, size_t, std::string&)
{
    return T_NOT_SUPPORTED;
}

int ProtoXmodem::TransmitFile(const std::string& filename, std::string& errmsg)
{
    int ret = TransmitFile(mConn->GetIo(), filename, errmsg);
    return ret;
}

int ProtoXmodem::TransmitData(const void* input, size_t input_size, std::string& errmsg)
{
    (void)input;
    (void)input_size;
    errmsg = "Not supported";
    return T_NOT_SUPPORTED;
}

int ProtoXmodem::TransmitFile()
{
    int ret = 0;
    FileSel fs;
    if (fs.AllFilesType().ExecuteOpen()) {
        mConn->Stop();
        auto filename = fs.Get();
        std::string errmsg;
        ret = TransmitFile(filename.ToStd(), errmsg);
        if (!errmsg.empty()) {
            PromptOK(errmsg.c_str());
        }
        mConn->Start();
    }
    return ret;
}

}
