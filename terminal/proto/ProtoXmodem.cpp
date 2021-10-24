//
// (c) 2020 chiv
//
#include "terminal_rc.h"
#include "ProtoXmodem.h"
#include "TransmitProgressDialog.h"
#include "xymodem.h"
#include "Conn.h"
#include "ProtoFactory.h"
#ifdef _MSC_VER
#include <chrono>
#else
// clang 10, there's a bug in <chrono>, so we use <sys/time.h>, gettimeofday instead.
#include <sys/time.h>
#endif
#include <thread>
#include <mutex>

namespace proto {

REGISTER_PROTO_INSTANCE("XMODEM", ProtoXmodem);

ProtoXmodem::ProtoXmodem(SerialConn* conn)
    : Proto(conn)
    , mXmodemK(false)
{
    WhenUsrBar = [=](Bar& bar) {
        bar.Add(t_("About"), terminal::help(), [=]() { Upp::PromptOK("Standard XMODEM/CRC v1.0a"); });
        bar.Add(t_("Transmit File..."), [=]() {
            TransmitFile();
        });
        bar.Add(t_("Enable XMODEM-k"), [=]() {
            mXmodemK = !mXmodemK;
        }).Check(mXmodemK);
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
    int timeout, volatile bool* should_stop, std::string& errmsg)
{
    bool failed = false;
    size_t blksz = std::min(frm_size, size_t(128));
    auto pkt = xymodem::Pack(frm, blksz, 128, xymodem::fSOH, xymodem::fEOF, cs_type, frm_idx);
    int retry = 3;
    while (retry--) {
        io->Write(pkt);
        // wait for the response
        int ret = expect_resp(io, timeout, should_stop, { xymodem::fACK, xymodem::fNAK, xymodem::fCAN });
        if (ret < 0) {
            errmsg = *should_stop ? "Aborted by user" : "The remote device was not responsed in time";
            failed = true;
            break;
        } else if (ret == xymodem::fNAK) { // retransmit
            io->Write(pkt);
        } else if (ret == xymodem::fACK) {
            break;
        } else if (ret == xymodem::fCAN) {
            errmsg = "The remote device canceled the transmit";
            failed = true;
            break;
        }
    }
    return failed ? -1 : (int)blksz;
}
static inline int _TransmitFrame1024(SerialIo* io, const void* frm, size_t frm_size, int cs_type, unsigned char frm_idx,
    int timeout, volatile bool* should_stop, std::string& errmsg)
{
    bool failed = false;
    size_t blksz = std::min(frm_size, size_t(1024));
    auto pkt = xymodem::Pack(frm, blksz, 1024, xymodem::fSTX, xymodem::fEOF, cs_type, frm_idx);
    int retry = 3;
    while (retry--) {
        io->Write(pkt);
        // wait for the response
        int ret = expect_resp(io, timeout, should_stop, { xymodem::fACK, xymodem::fNAK, xymodem::fCAN });
        if (ret < 0) {
            errmsg = *should_stop ? "Aborted by user" : "The remote device was not responsed in time";
            failed = true;
            break;
        } else if (ret == xymodem::fNAK) { // retransmit
            io->Write(pkt);
        } else if (ret == xymodem::fACK) {
            break;
        } else if (ret == xymodem::fCAN) {
            errmsg = "The remote device canceled the transmit";
            failed = true;
            break;
        }
    }
    return failed ? -1 : (int)blksz;
}
// return tv1 - tv2 in seconds
static inline double _diff(const struct timeval& tv1, const struct timeval& tv2)
{
    double d_secs = tv1.tv_sec - tv2.tv_sec;
    double d_usecs = tv1.tv_usec - tv2.tv_usec;
    return d_secs + d_usecs / 1000000.;
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
        if (filesz > xymodem::kMaxFileSize) {
            errmsg = filename + " is too large to transmit, max:" + std::to_string(xymodem::kMaxFileSize);
            return -1;
        }
        bar.SetTotal(filesz);
        //
        std::mutex lock;
        double ts = 0;
        size_t count = 0;
        double rate = 0;
        // create the job
        auto job = [&]() {
            unsigned char idx = 1; // xymodem begins from 1
            // wait for sync
            int cs_type = xymodem::CRC16;
            int resp = expect_resp(io, xymodem::kSyncTimeout, &should_stop, { 'C', xymodem::fNAK, 'W' });
            switch (resp) {
            case 'C': cs_type = xymodem::CRC16; break;
            case 'W': failed = true; errmsg = "WX-Modem CRC was not supported!"; break;
            case xymodem::fNAK: cs_type = xymodem::CSUM; break;
            case -1: failed = true; errmsg = "Unrecognized sync character!"; break;
            default: failed = true; errmsg = "Sync Timeout!"; break;
            }
            //
            char chunk[1024];
            while (!should_stop && !failed && !fin.IsEof()) {
                auto frmsz = fin.Get(chunk, mXmodemK ? 1024 : 128);
#ifdef _MSC_VER
                auto t1 = std::chrono::high_resolution_clock::now();
#else
                struct timeval t1, t2;
                gettimeofday(&t1, NULL);
#endif
                if (mXmodemK) {
                    if (_TransmitFrame1024(io, chunk, frmsz, cs_type, idx++, xymodem::kTimeout, &should_stop, errmsg) < 0) {
                        failed = true;
                        break;
                    }
                } else if (_TransmitFrame128(io, chunk, frmsz, cs_type, idx++, xymodem::kTimeout, &should_stop, errmsg) < 0) {
                    failed = true;
                    break;
                }
                // update progress bar
                if (!failed) {
                    std::lock_guard<std::mutex> _(lock);
                    count += frmsz;
#ifdef _MSC_VER
                    ts += std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t1).count();
#else
                    gettimeofday(&t2, NULL);
                    ts += _diff(t2, t1); // t2 - t1
#endif
                    rate = count / ts;
                    PostCallback([&]() {
                        std::lock_guard<std::mutex> _(lock);
                        bar.Update(count, rate);
                    });
                }
            }
            // write the tail seq
            if (!failed) {
                failed = true;
                io->Write((const unsigned char*)&xymodem::fEOT, 1);
                if (expect_resp(io, xymodem::kTimeout, &should_stop, { xymodem::fACK }) == xymodem::fACK) {
                    //io->Write((const unsigned char*)&xymodem::fETB, 1);
                    //if (expect_resp(io, xymodem::kTimeout, &should_stop, {xymodem::fACK}) == xymodem::fACK) {
                    // completed.
                    failed = false;
                    //} else {
                    //    errmsg = "ETB was not responsed!";
                    //}
                } else {
                    errmsg = "EOT was not responsed!";
                }
            }
            if (should_stop == true) io->Write(std::vector<unsigned char>({xymodem::fCAN}));
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
    errmsg = "XMODEM only support file transmit!";
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
