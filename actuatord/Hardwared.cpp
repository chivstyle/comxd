//
// (c) 2021 chiv
//
#include "Hardwared.h"
#include <chrono>
#include <thread>

using namespace Upp;

// 0x68,0x68,type[1byte],order[1byte],[64bytes],cs[1byte],tail[1byte]
static const size_t kFrameSize = 70;
static const size_t kFrameDataSize = 64;

#define TAG    "<Hardwared>:"

Hardwared::Hardwared(const std::string& devname, uint32_t baudrate)
{
	mIo = new serial::Serial(devname, baudrate);
}

Hardwared::Hardwared(const Value& conf)
{
    const Value& s = conf["Serial"];
	mIo = new serial::Serial(s["Name"].ToStd(),
							 static_cast<uint32_t>((int)s["Baudrate"]),
							 serial::Timeout(),
							 static_cast<serial::bytesize_t>((int)s["ByteSize"]),
							 static_cast<serial::parity_t>((int)s["Parity"]),
							 static_cast<serial::stopbits_t>((int)s["StopBits"]),
							 static_cast<serial::flowcontrol_t>((int)s["FlowControl"]));
}

bool Hardwared::IsValidFrame(const std::vector<unsigned char>& frame)
{
    if (frame.size() != kFrameSize) {
        LOG(TAG << "frame size is " << frame.size() << ", expect " << kFrameSize);
        return false;
    }
    if (frame[0] != 0x68 || frame[1] != 0x68) {
        LOG(TAG << "head error [" << frame[0] << "," << frame[1] << " expect [" << 0xfe << 0xfe << "]");
        return false;
    }
    if (*frame.rbegin() != 0xfe) {
        LOG(TAG << "tail error " << *frame.rbegin() << " expect " << 0xfe);
        return false;
    }
    // cs
    size_t cs = 0;
    for (size_t k = 0; k < frame.size() - 2; ++k) {
        cs += frame[k];
    }
    cs = cs & 0xff;
    if (cs != frame[frame.size() - 2]) {
        LOG(TAG << "cs error " << frame[frame.size() - 2] << " expect " << cs);
        return false;
    }
    // reach here, it's a valid frame.
    return true;
}

// Make frame that MCU supported
// sz should be 64, if > 64, truncated, else if < 64, padding with 0.
std::vector<unsigned char> Hardwared::MakeFrame(unsigned char type, unsigned char order,
                                                const unsigned char* data, size_t sz)
{
    std::vector<unsigned char> out(kFrameSize, 0);
    size_t cs = 0x68*2 + type + order;
    out[0] = 0x68; out[1] = 0x68;
    out[2] = type; out[3] = order;
    for (size_t k = 0; k < sz && k < kFrameDataSize; ++k) {
        out[4 + k] = data[k];
        cs += data[k];
    }
    out[out.size() - 2] = (unsigned char)(cs & 0xff);
    out[out.size() - 1] = 0xfe;
    return out; // implies std::move
}

std::vector<unsigned char> Hardwared::MakeFrame(unsigned char type, unsigned char order,
                                                const std::vector<unsigned char>& data)
{
    return MakeFrame(type, order, data.data(), data.size());
}

Hardwared::~Hardwared()
{
	delete mIo;
}

bool Hardwared::SendFrame(const std::vector<unsigned char>& frame,
                          std::vector<unsigned char>& response,
                          int timeout, volatile bool* should_exit)
{
    std::unique_lock<std::mutex> _(mLock);
    mResponseCount = 0;
    mIo->write(frame);
    LOG(TAG << "Send frame to device");
    DUMPHEX(String((const char*)frame.data(), frame.size()));
    bool ret = mCond.wait_for(_, std::chrono::milliseconds(timeout), [=]() {
        return mResponseCount > 0 || (should_exit && *should_exit);
    });
    if (mResponseCount > 0) { // received response from device
        response = mResponse;
    }
    return mResponseCount > 0;
}

void Hardwared::Run(volatile bool* should_exit)
{
    std::vector<unsigned char> buff;
    bool pending = false;
	while (*should_exit == false) {
		size_t asz = mIo->available();
		if (asz == 0) {
		    // wait for a while
		    std::this_thread::sleep_for(std::chrono::milliseconds(20));
		} else {
		    if (pending) {
		        size_t expect_cnt = kFrameSize - buff.size();
		        mIo->read(buff, std::min(expect_cnt, asz));
		        if (buff.size() == kFrameSize) {
		            pending = false; // got a frame
		            //
		            LOG(TAG << "Received frame");
		            DUMPHEX(String((const char*)buff.data(), buff.size()));
		            // Notify
		            std::lock_guard<std::mutex> _(mLock);
		            // check and process
		            if (IsValidFrame(buff)) {
		                mResponseCount++;
		                mResponse = std::move(buff);
		                //
		                mCond.notify_all();
		            } else {
		                LOG(TAG << "Invalid Response");
		            }
		        }
		    } else {
		        mIo->read(buff, 1);
		        if (buff.size() == 2 && buff[0] == 0x68 && buff[1] == 0x68) {
		            pending = true; // maybe there's a valid frame in buffer, let's go
		        } else if (buff.size() == 2) { // it's not a valid header, go on
		            LOG(TAG << "Received invalid frame head, reagain");
		            DUMPHEX(String((const char*)buff.data(), buff.size()));
		            buff.erase(buff.begin());
		        }
		    }
		}
	}
}
