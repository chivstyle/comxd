//
// (c) 2020 chiv
//
#include "terminal_rc.h"
#include "ProtoYmodem.h"
#include "TransmitProgressDialog.h"
#include "xyzmodem.h"
#include "Conn.h"
#include "ProtoFactory.h"
#include <thread>

namespace proto {
REGISTER_PROTO_INSTANCE("YMODEM", ProtoYmodem);

ProtoYmodem::ProtoYmodem(SerialConn* conn)
    : Proto(conn)
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
    return t_("Standard YMODEM-1K/CRC");
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

static inline std::string _Filename(const std::string& pathname)
{
    int len = (int)pathname.length();
    while (pathname[len] != '/' && pathname[len] != '\\') {
        len--;
    }
    // len >= 0, found 1 path prefix at least
    return len >= 0 ? pathname.substr(len + 1) : pathname;
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
        if (filesz > xyzmodem::kMaxFileSize) {
            errmsg = filename + " is too large to transmit, max:" + std::to_string(xyzmodem::kMaxFileSize);
            return -1;
        }
        bar.SetTotal(filesz);
        //
        size_t count = 0;
        double ts = 0;
        // create the job
        auto job = [&]() {
            unsigned char idx = 1; // xyzmodem begins from 1
            // wait for sync
            int resp = expect_resp(io, xyzmodem::kTimeout, &should_stop, { 'C' });
            switch (resp) {
            case 'C': break;
            case -1: failed = true; errmsg = "Sync Timeout!"; break;
            default: failed = true; errmsg = "Unrecognized sync character!"; break;
            }
            // send filename
            auto filename_ = _Filename(filename);
            auto hdr = xyzmodem::Pack(filename_.c_str(), std::min(size_t(127), filename_.length()), 128,
                xyzmodem::fSOH, '\0', xyzmodem::CRC16, 0);
            io->Write(hdr);
            if (!expect_seqs(io, xyzmodem::kTimeout, &should_stop, { xyzmodem::fACK, 'C' })) {
                failed = true;
                errmsg = "Failed to transmit filename";
            }
            //
            auto t1 = std::chrono::high_resolution_clock::now();
            char chunk[1024];
            while (!should_stop && !failed && !fin.IsEof()) {
                auto frmsz = fin.Get(chunk, 1024);
                if (_TransmitFrame1024(io, chunk, frmsz, xyzmodem::CRC16, idx++, &should_stop, errmsg) < 0) {
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
                io->Write((unsigned char*)&xyzmodem::fEOT, 1);
                if (expect_resp(io, xyzmodem::kTimeout, &should_stop, {xyzmodem::fNAK}) != xyzmodem::fNAK) {
                    failed = true;
                    errmsg = "EOT 1 was not responsed by NAK";
                } else {
                    io->Write((unsigned char*)&xyzmodem::fEOT, 1);
                    if (expect_seqs(io, xyzmodem::kTimeout, &should_stop, {xyzmodem::fACK})) {
                        failed = false;
                        if (last_one) {
                            io->Write(xyzmodem::Pack(nullptr, 0, 128, xyzmodem::fSOH, '\0', xyzmodem::CRC16, 0));
                            if (expect_seqs(io, xyzmodem::kTimeout, &should_stop, {xyzmodem::fACK})) {
                                failed = false;
                            } else {
                                failed = true;
                                errmsg = "TAIL was not responsed by ACK";
                            }
                        }
                    } else {
                        errmsg = "EOT 2 was not responsed by ACK";
                    }
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
    errmsg = "Not supported";
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
