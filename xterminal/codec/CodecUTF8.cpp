//
// (c) 2020 chiv
//
#include "CodecUTF8.h"
#include "CodecFactory.h"

REGISTER_CODEC_INSTANCE("UTF-8", CodecUTF8);

std::vector<uint32_t> CodecUTF8::TranscodeToUTF32(const unsigned char* data, size_t sz, size_t* ep)
{
	size_t ep_;
    std::vector<uint32_t> out = UTF8ToUTF32_(data, sz, ep_);
    if (ep) *ep = ep_;
    return std::move(out);
}

std::string CodecUTF8::TranscodeToUTF8(const unsigned char* data, size_t sz, size_t* ep)
{
    std::string out;
    out.resize(sz);
    std::copy(data, data + sz, out.begin());
    if (ep) *ep = sz;
    return std::move(out);
}

std::string CodecUTF8::TranscodeFromUTF8(const unsigned char* data, size_t sz, size_t* ep)
{
    std::string out;
    out.resize(sz);
    std::copy(data, data + sz, out.begin());
    if (ep) *ep = sz;
    return std::move(out);
}

std::string CodecUTF8::TranscodeFromUTF32(const uint32_t* data, size_t sz)
{
    std::string out;
    for (size_t k = 0; k < sz; ++k) {
        out += UTF32ToUTF8_(data[k]);
    }
    return out;
}
