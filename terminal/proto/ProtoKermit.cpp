//
// (c) 2021 chiv
//
#include "terminal_rc.h"
#include "ProtoKermit.h"
#include "TransmitProgressDialog.h"
#include "xymodem.h"
#include "Conn.h"
#include "ProtoFactory.h"
#include <thread>

namespace proto {
REGISTER_PROTO_INSTANCE("KERMIT", ProtoKermit);
namespace kermit {
    static const int kMaxKermitFrameSize = 223*95 + 95;
    static const int kFrameSize = 4096;
    static_assert(kFrameSize < kMaxKermitFrameSize, "kFrameSize must be less than kMaxKermitFrameSize");
    // data - encoded data with kermit
    // data_sz - size of data, in bytes, must be < kMaxKermitFrameSize
    // mark - frame type
    std::string Pack(char mark, const char* data, size_t data_sz, unsigned char idx)
    {
        std::string out;
        out.push_back(xymodem::fSOH);
        idx = idx % 64;
        // len include all chars following it
        if (data_sz <= 220) { // len : seq [ 1 byte ] + mark [ 1 byte ] + data [ data_sz ] + checksum [ 1 byte ]
            size_t len = data_sz + 3;
            out.push_back((char)len + 32); out.push_back(idx + 32); out.push_back(mark);
        } else if (data_sz <= kMaxKermitFrameSize - 1)  {
            // len: data_sz + checkusm [ 1 byte ]
            out.push_back(32); out.push_back(idx + 32); out.push_back(mark);
            size_t len = data_sz + 1;
            out.push_back((char)(len / 95) + 32);
            out.push_back((char)(len % 95) + 32);
            // header sum
            size_t header_sum = 0;
            for (size_t k = 1; k < out.size(); ++k) {
                header_sum += out[k];
            }
            out.push_back((char)((header_sum + ((header_sum >> 6) & 0x03)) & 0x3f) + 32);
        } else return out;
        // push data
        for (size_t k = 0; k < data_sz; ++k) {
            out.push_back((char)data[k]);
        }
        // checksum
        size_t sum = 0;
        for (size_t k = 1; k < out.size(); ++k) {
            sum += out[k];
        }
        out.push_back((char)((sum + ((sum >> 6) & 0x03)) & 0x3f) + 32);
        out.push_back(0xd);
        //
        return out;
    }
    //
    struct KermitMsg {
        char Mark;
        unsigned char Seq;
        std::vector<unsigned char> Data;
        KermitMsg()
            : Mark(0)
            , Seq(0)
        {
        }
    };
    size_t Parse(const unsigned char* data, size_t data_sz, KermitMsg& msg)
    {
        // SOH length mark data checksum
        if (data_sz > 0 && data[0] == xymodem::fSOH) {
            // pending
            if (data_sz > 4) {
                size_t sum = 0;
                int len = data[1] - 32; sum += data[1];
                unsigned char seq = data[2] - 32; --len; sum += data[2];
                // mark is not encoded
                int mark = data[3]; --len; sum += data[3];
                int dpp = 4;
                if (len == -2 && data_sz > 6) {
                    len = (data[4] - 32) * 95 + data[5];
                    sum += data[4] + data[5];
                    dpp = 7;
                    // checksum of header
                    unsigned char hdr_sum = (unsigned char)((sum + ((sum >> 6) & 0x03)) & 0x3f) + 32;
                    if (hdr_sum != data[6])
                        return 0; // checksum of header was wrong.
                }
                std::vector<unsigned char> out;
                if (len < data_sz - dpp) {
                    int p = 0;
                    bool escape = false;
                    while (p < len-1) {
                        if (escape) {
                            if (data[dpp + p] == '#')
                                out.push_back('#');
                            else
                                out.push_back(data[dpp + p] ^ 64);
                        } else {
                            if (data[dpp + p] == '#') {
                                escape = true;
                            } else {
                                escape = false;
                                out.push_back(data[dpp + p]);
                            }
                        }
                        sum += data[dpp + p];
                        p++;
                    }
                    sum = (unsigned char)((sum + ((sum >> 6) & 0x03)) & 0x3f) + 32;
                    if (sum == data[dpp + len-1]) {
                        // ok
                        msg.Data = std::move(out);
                        msg.Mark = mark;
                        msg.Seq = seq;
                        //
                        return dpp + len;
                    }
                }
            }
        }
        return 0;
    }
    //
    std::string Encode(const void* data, size_t data_sz)
    {
        std::string out;
        unsigned char* p = (unsigned char*)data;
        for (size_t k = 0; k < data_sz; ++k) {
            if (p[k] < 32) {
                out.push_back('#');
                out.push_back(p[k] ^ 64);
            } else if (p[k] == '#') {
                out.push_back('#');
                out.push_back('#');
            } else {
                out.push_back(p[k]);
            }
        }
        return out;
    }
    //
    class KermitDataStream {
    public:
        KermitDataStream(const std::string& fname, int chunk_size)
            : mChunkSize(chunk_size)
            , mBytesRead(0)
        {
            if (mIn.Open(fname.c_str())) {
                mFileSize = mIn.GetSize();
            } else
                mFileSize = 1;
        }
        ~KermitDataStream()
        {
            mIn.Close();
        }
        
        size_t FileSize() { return mFileSize; }
        
        void Step()
        {
            if (!mIn.IsEof()) {
                std::vector<unsigned char> buff(mChunkSize);
                int asz = mIn.Get(buff.data(), mChunkSize);
                // data frame
                mStream += Encode(buff.data(), asz);
                //
                mBytesRead += asz;
            }
        }
        // Take will step the file
        std::string Take(int sz)
        {
            std::string out;
            //
            Step();
            //
            if (sz > mStream.length()) {
                std::swap(out, mStream);
            } else {
                out = mStream.substr(0, sz);
                mStream = mStream.substr(sz);
            }
            return out;
        }
        
        bool IsEnd()
        {
            return mStream.empty() && mIn.IsEof();
        }
        //
        size_t BytesRead() const { return mBytesRead; }
        
    private:
        Upp::FileIn mIn;
        int mChunkSize;
        //
        size_t mFileSize;
        size_t mBytesRead;
        //
        std::string mStream;
    };
    
    static int kMaxFileSz = 64*1024*1024;
    static int kTimeout = 10000;
}

ProtoKermit::ProtoKermit(SerialConn* conn)
    : Proto(conn)
{
}

std::string ProtoKermit::GetDescription() const
{
    return "KERMIT v1.0a by chiv";
}
// find sequence from response, return 1 for matched seq, others for failure
static inline kermit::KermitMsg expect_resp(SerialIo* io, int timeout, volatile bool* should_stop)
{
    std::string buff;
    bool pending = false;
    while (timeout >= 0 && !*should_stop) {
        int sz = io->Available();
        if (sz < 0)
            break; // The IO device was corrupted
        if (sz > 0) {
            unsigned char c;
            io->Read(&c, 1); // read one byte
            if (pending == false && c == xymodem::fSOH) {
                pending = true;
            }
            if (pending) {
                buff.push_back((char)c);
                kermit::KermitMsg msg;
                int ret = kermit::Parse((unsigned char*)buff.c_str(), buff.length(), msg);
                if (ret > 0) {
                    return msg;
                }
            }
        } else
            std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
        //
        timeout -= 10;
    }
    return kermit::KermitMsg();
}

static void _FlushIn(SerialIo* io)
{
    int asz = io->Available();
    if (asz > 0)
        io->ReadRaw(asz);
}

static bool _TransmitFrame(SerialIo* io, const std::string& b_data, std::string& errmsg,
    kermit::KermitMsg& msg, volatile bool* should_stop)
{
    int retry = 3;
    bool failed = false;
    bool ack = false;
    while (retry-- && !failed && !ack) {
        io->Write(b_data);
        msg = expect_resp(io, kermit::kTimeout, should_stop);
        switch (msg.Mark) {
        case 'Y': ack = true; break;
        case 'N': break; // NAK, break the switch to retry
        case 'E':
            failed = true;
            errmsg.insert(errmsg.begin(), msg.Data.begin(), msg.Data.end());
            break;
        default:
            failed = true;
            errmsg = "DATA FRAME was not responsed by ACK";
            break;
        }
    }
    if (retry < 0) {
        failed = true;
        errmsg = "Too many retries";
    }
    return failed;
}

int ProtoKermit::TransmitFile(SerialIo* io, const std::string& fname, std::string& errmsg, bool last_file)
{
    TransmitProgressDialog bar;
    bool failed = false;
    volatile bool should_stop = false;
    int  total = 0;
    kermit::KermitDataStream fin(fname, 1024);
    bar.WhenClose = [&]() { should_stop = true; };
    bar.SetTotal(fin.FileSize());
    bar.Title(fname.c_str());
    if (fin.FileSize() > kermit::kMaxFileSz) {
        failed = true;
        errmsg = "You should not transmit big files by kermit!";
    } else {
        total = (int)fin.FileSize();
        double ts = 0;
        auto job = [&]() {
            unsigned char idx = 0;
            kermit::KermitMsg msg;
            // 0. init
            _FlushIn(io);
            // 1. init
            auto b_init = kermit::Pack('S', nullptr, 0, idx++);
            io->Write(b_init);
            msg = expect_resp(io, kermit::kTimeout, &should_stop);
            if (msg.Mark != 'Y') {
                failed = true;
                errmsg = "INIT was not responsed by ACK";
            }
            // 2. file header
            if (!failed) {
                auto basename = xymodem::_Filename(fname);
                auto b_file = kermit::Pack('F', basename.c_str(), basename.length(), idx++);
                io->Write(b_file);
                msg = expect_resp(io, kermit::kTimeout, &should_stop);
                if (msg.Mark != 'Y') {
                    failed = true;
                    errmsg = "FILE HEADER was not responsed by ACK";
                }
            }
            // 3. data
            auto t1 = std::chrono::high_resolution_clock::now();
            // for retry
            unsigned char i_last = 255;
            std::string b_last;
            //
            while (!fin.IsEnd() && !failed && !should_stop) {
                auto data = fin.Take(kermit::kFrameSize);
                if (data.empty()) break;
                auto b_data = kermit::Pack('D', data.c_str(), data.length(), idx++);
                failed = _TransmitFrame(io, b_data, errmsg, msg, &should_stop);
#if 0
                // wait until the device accepted last frame.
                while (!failed) {
                    failed = _TransmitFrame(io, b_data, errmsg, msg, &should_stop);
                    if (!failed) {
                        if (msg.Seq != i_last) {
                            i_last = msg.Seq;
                            b_last = b_data;
                            // the last frame was accepted by the device, so we break this loop
                            // to send the next frame.
                            break;
                        } else {
                            failed = _TransmitFrame(io, b_last, errmsg, msg, &should_stop);
                            if (!failed) {
                                // reset the i_last
                                i_last = msg.Seq;
                            }
                        }
                    }
                }
#endif
                // update progress bar
                if (!failed) {
                    ts = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t1).count();
                    PostCallback([&]() { bar.Update(fin.BytesRead(), (double)fin.BytesRead() / ts); });
                }
            }
            // 4. EOT
            if (!failed) {
                auto b_eot = kermit::Pack('Z', nullptr, 0, idx++);
                io->Write(b_eot);
                if (expect_resp(io, kermit::kTimeout, &should_stop).Mark != 'Y') {
                    failed = true;
                    errmsg = "EOT was not responsed by ACK";
                }
            }
            // 5. BREAK
            if (last_file && !failed) {
                auto b_brk = kermit::Pack('B', nullptr, 0, idx++);
                io->Write(b_brk);
                if (expect_resp(io, 200, &should_stop).Mark != 'Y') {
                    failed = true;
                    errmsg = "BREAK was not responsed by ACK";
                }
            }
            if (should_stop) {
                io->Write(kermit::Pack('E', "User Cancel", strlen("User Cancel"), idx++));
            }
            //
            Upp::PostCallback([&]() { bar.Close(); });
        };
        auto thr = std::thread(job);
        bar.RunAppModal();
        thr.join();
    }
    return !failed ? (int)total : -1;
}

int ProtoKermit::TransmitFile()
{
    int ret = 0;
    FileSel fs;
    if (fs.AllFilesType().Multi(true).ExecuteOpen()) {
        mConn->Stop();
        size_t cnt = 0;
        for (int k = 0; k < fs.GetCount(); ++k) {
            std::string errmsg;
            ret = TransmitFile(mConn->GetIo(), fs.GetFile(k).ToStd(), errmsg, k == fs.GetCount() - 1);
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
