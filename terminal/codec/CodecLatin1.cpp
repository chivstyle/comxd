//
// (c) 2020 chiv
//
#include "CodecLatin1.h"
#include "CodecFactory.h"
#include <CtrlLib/CtrlLib.h>

REGISTER_CODEC_INSTANCE("Latin1 (ISO-8859-1)", CodecLatin1);

std::vector<uint32_t> CodecLatin1::TranscodeToUTF32(const unsigned char* data, size_t sz, size_t& ep)
{
    std::vector<uint32_t> out(sz);
    std::copy(data, data + sz, out.begin());
    ep = sz;
    return std::move(out);
}

std::string CodecLatin1::TranscodeToUTF8(const unsigned char* data, size_t sz)
{
    Upp::String raw(data, (int)sz);
    Upp::ToCharset(Upp::CHARSET_ISO8859_1, raw, CHARSET_UTF8);
    return raw.ToStd();
}

std::string CodecLatin1::TranscodeFromUTF8(const unsigned char* data, size_t sz)
{
    Upp::String raw(data, (int)sz);
    return Upp::ToCharset(Upp::CHARSET_ISO8859_1, raw, CHARSET_UTF8).ToStd();
}

std::string CodecLatin1::TranscodeFromUTF32(const uint32_t* data, size_t sz)
{
    std::string out;
    out.resize(sz);
    for (size_t k = 0; k < out.length(); ++k) {
        out[k] = (char)data[k];
    }
    return std::move(out);
}
