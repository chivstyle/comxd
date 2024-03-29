//
// (c) 2020 chiv
//
#include "terminal_rc.h"
#include "ProtoYmodem.h"
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

REGISTER_PROTO_INSTANCE("YMODEM-1K", ProtoYmodem);

ProtoYmodem::ProtoYmodem(SerialConn* conn)
    : Proto(conn)
    , mType(eYmodem1K)
{
    WhenUsrBar = [=](Bar& bar) {
        bar.Add(t_("About"), terminal::help(), [=]() {
            Upp::PromptOK("Standard YMODEM-1K/CRC v1.0a");
        });
        bar.Add(t_("Transmit Files..."), [=]() {
            TransmitFile();
        });
    };
}

ProtoYmodem::~ProtoYmodem()
{
}

std::string ProtoYmodem::GetDescription() const
{
    std::string desc;
    switch (mType) {
    case eStandardYmodem:
        desc = t_("Standard YMODEM/CRC");
        break;
    default:
        mType = eYmodem1K;
        desc = t_("Standard YMODEM-1K/CRC");
        break;
    }
    return desc;
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
template <int FSZ = 1024, int FLAG = xymodem::fSTX>
static inline int _TransmitFrame(SerialIo* io, const void* frm, size_t frm_size, int cs_type, unsigned char frm_idx,
    int timeout, volatile bool* should_stop, std::string& errmsg)
{
    bool failed = false;
    size_t blksz = std::min(frm_size, size_t(FSZ));
    auto pkt = xymodem::Pack(frm, blksz, FSZ, (unsigned char)FLAG, xymodem::fEOF, cs_type, frm_idx);
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

int ProtoYmodem::TransmitFile(SerialIo* io, const std::string& filename, std::string& errmsg, bool last_one)
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
        int64 count = 0;
        double ts = 0;
        double rate = 0;
        // create the job
        auto job = [&]() {
            unsigned char idx = 1; // xymodem begins from 1
            // wait for sync
            int resp = expect_resp(io, xymodem::kSyncTimeout, &should_stop, { 'C', xymodem::fCAN });
            switch (resp) {
            case 'C': break;
            case -1: failed = true; errmsg = "Sync Timeout!"; break;
            case xymodem::fCAN: failed = true; errmsg = "Canceled by remote peer!"; break;
            default: failed = true; errmsg = "Unrecognized sync character!"; break;
            }
            // send filename
            auto filename_ = xymodem::_Filename(filename);
            std::vector<unsigned char> finfo;
            finfo.insert(finfo.end(), filename_.begin(), filename_.end());
            finfo.push_back('\0');
            std::string fsz_ = std::to_string(filesz);
            finfo.insert(finfo.end(), fsz_.begin(), fsz_.end());
            auto hdr = xymodem::Pack(finfo.data(), std::min(size_t(127), finfo.size()), 128,
                xymodem::fSOH, '\0', xymodem::CRC16, 0);
            io->Write(hdr);
            if (!expect_seqs(io, xymodem::kSyncTimeout, &should_stop, { xymodem::fACK, 'C' })) {
                failed = true;
                errmsg = "Failed to transmit filename";
            }
            while (!should_stop && !failed && !fin.IsEof()) {
                int64 frmsz = 0;
#ifdef _MSC_VER
                auto t1 = std::chrono::high_resolution_clock::now();
#else
                struct timeval t1, t2;
                gettimeofday(&t1, NULL);
#endif
                if (mType == eStandardYmodem) {
                    char chunk[128];
                    frmsz = fin.Get(chunk, 128);
                    if (_TransmitFrame<128, xymodem::fSOH>(io, chunk, (size_t)frmsz, xymodem::CRC16, idx++, xymodem::kTimeout,
                        &should_stop, errmsg) < 0) {
                        failed = true;
                        break;
                    }
                } else {
                    char chunk[1024];
                    frmsz = fin.Get(chunk, 1024);
                    if (_TransmitFrame<1024, xymodem::fSTX>(io, chunk, (size_t)frmsz, xymodem::CRC16, idx++, xymodem::kTimeout,
                        &should_stop, errmsg) < 0) {
                        failed = true;
                        break;
                    }
                }
                if (!failed) {
                    std::lock_guard<std::mutex> _(lock);
                    count += (size_t)frmsz;
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
                io->Write((unsigned char*)&xymodem::fEOT, 1);
                if (expect_seqs(io, xymodem::kSyncTimeout, &should_stop, {xymodem::fACK, 'C'})) {
                    failed = false;
                    if (last_one) {
                        io->Write(xymodem::Pack(nullptr, 0, 128, xymodem::fSOH, '\0', xymodem::CRC16, 0));
                        if (expect_seqs(io, xymodem::kTimeout, &should_stop, {xymodem::fACK})) {
                            failed = false;
                        } else {
                            failed = true;
                            errmsg = "TAIL was not responsed by ACK";
                        }
                    }
                } else {
                    errmsg = "EOT was not responsed by ACK.C";
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

int ProtoYmodem::TransmitData(SerialIo*, const void*, size_t, std::string&)
{
    return T_NOT_SUPPORTED;
}

int ProtoYmodem::TransmitFile(const std::string& filename, std::string& errmsg, bool last_one)
{
    int ret = TransmitFile(mConn->GetIo(), filename, errmsg, last_one);
    return ret;
}
int ProtoYmodem::TransmitData(const void* input, size_t input_size, std::string& errmsg)
{
    (void)input;
    (void)input_size;
    errmsg = "YMODEM only support file(s) transmit!";
    return T_NOT_SUPPORTED;
}

int ProtoYmodem::TransmitFile()
{
    int ret = 0;
    FileSel fs;
    if (fs.AllFilesType().Multi(true).ExecuteOpen()) {
        mConn->Stop();
        size_t cnt = 0;
        for (int k = 0; k < fs.GetCount(); ++k) {
            std::string errmsg;
            ret = TransmitFile(fs.GetFile(k).ToStd(), errmsg, k == fs.GetCount() - 1);
            if (!errmsg.empty()) {
                PromptOK(errmsg.c_str());
            }
            if (ret <= 0)
                break;
            cnt++;
        }
        mConn->Start();
    }
    return ret;
}

}
