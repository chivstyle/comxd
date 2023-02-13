//
// (c) 2020 chiv
//
#include "CodecGB18030.h"
#include "CodecFactory.h"
#ifdef _WIN32
#include <Windows.h>
#if WINVER >= 0x0A00
#if NTDDI_VERSION >= NTDDI_WIN10_19H1
#include <icu.h>

REGISTER_CODEC_INSTANCE("GB18030", CodecGB18030);

std::vector<uint32_t> CodecGB18030::TranscodeToUTF32(const unsigned char* data, size_t sz, size_t& ep)
{
	UErrorCode ec = U_ZERO_ERROR;
	std::vector<uint32_t> s;
	ep = 0;
    int32_t rc = ucnv_convert("UTF32", "GB18030", (char*)s.data(), 0, (const char*)data, (int32_t)sz, &ec);
    if (ec == U_BUFFER_OVERFLOW_ERROR) {
        s.resize(rc / 4);
        ec = U_ZERO_ERROR;
        rc = ucnv_convert("UTF32", "GB18030", (char*)s.data(), (int32_t)(sizeof(uint32_t)*s.size()), (const char*)data, (int32_t)sz, &ec);
        if (rc > 0) {
            s.erase(s.begin()); // remove the 'Non character( 0xfffe )'
            ep = sz; // TODO: how to process the incomplete character sequences
            return s;
        }
    }
    return std::vector<uint32_t>(1, '?');
}
// TODO: how to process the incomplete character sequences
std::string CodecGB18030::TranscodeToUTF8(const unsigned char* data, size_t sz)
{
	UErrorCode ec = U_ZERO_ERROR;
	std::vector<char> s;
    int32_t rc = ucnv_convert("UTF8", "GB18030", s.data(), 0, (const char*)data, (int32_t)sz, &ec);
    if (ec == U_BUFFER_OVERFLOW_ERROR) {
        s.resize(rc + 1);
        ec = U_ZERO_ERROR;
        rc = ucnv_convert("UTF8", "GB18030", s.data(), (int32_t)s.size(), (const char*)data, (int32_t)sz, &ec);
        if (rc > 0) {
            s[rc] = '\0';
            return s.data();
        }
    }
    return "?";
}
// TODO: how to process the incomplete character sequences
std::string CodecGB18030::TranscodeFromUTF8(const unsigned char* data, size_t sz)
{
	UErrorCode ec = U_ZERO_ERROR;
	std::vector<char> s;
    int32_t rc = ucnv_convert("GB18030", "UTF8", s.data(), 0, (const char*)data, (int32_t)sz, &ec);
    if (ec == U_BUFFER_OVERFLOW_ERROR) {
        s.resize(rc + 1);
        ec = U_ZERO_ERROR;
        rc = ucnv_convert("GB18030", "UTF8", s.data(), (int32_t)s.size(), (const char*)data, (int32_t)sz, &ec);
        if (rc > 0) {
            s[rc] = '\0';
            return s.data();
        }
    }
    return "?";
}
// TODO: how to process the incomplete character sequences
std::string CodecGB18030::TranscodeFromUTF32(const uint32_t* data, size_t sz)
{
	UErrorCode ec = U_ZERO_ERROR;
	std::vector<char> s;
    int32_t rc = ucnv_convert("GB18030", "UTF32", s.data(), 0, (const char*)data, (int32_t)(sizeof(uint32_t)*sz), &ec);
    if (ec == U_BUFFER_OVERFLOW_ERROR) {
        s.resize(rc + 1);
        ec = U_ZERO_ERROR;
        rc = ucnv_convert("GB18030", "UTF32", s.data(), (int32_t)s.size(), (const char*)data, (int32_t)(sizeof(uint32_t)*sz), &ec);
        if (rc > 0) {
            s[rc] = '\0';
            return s.data();
        }
    }
    return "?";
}
#endif
#endif
#endif

