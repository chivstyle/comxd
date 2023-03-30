//
// (c) 2020 chiv
//
#include "CodecGB18030.h"
#include "CodecFactory.h"

#ifndef WITHOUT_ICU

#ifdef _WIN32
#include <Windows.h>
#if WINVER >= 0x0A00
#if NTDDI_VERSION >= NTDDI_WIN10_19H1
#include <icu.h>

REGISTER_CODEC_INSTANCE("GB18030", CodecGB18030);

CodecGB18030::CodecGB18030()
{
	UErrorCode ec = U_ZERO_ERROR;
    mUtf8Cnv = ucnv_open("UTF8", &ec);
    ec = U_ZERO_ERROR;
    mGb18030Cnv = ucnv_open("GB18030", &ec);
}

CodecGB18030::~CodecGB18030()
{
	if (mUtf8Cnv) {
		ucnv_close(mUtf8Cnv);
	}
	if (mGb18030Cnv) {
		ucnv_close(mGb18030Cnv);
	}
}

std::vector<uint32_t> CodecGB18030::TranscodeToUTF32(const unsigned char* data, size_t sz, size_t* ep)
{
	std::vector<uint32_t> out;
	if (ep) *ep = sz;
	if (mGb18030Cnv) {
		UErrorCode ec = U_ZERO_ERROR;
		const char* in = (const char*)data;
		UChar32 uc = ucnv_getNextUChar(mGb18030Cnv, &in, (const char*)data + sz, &ec);
		while (ec != U_INDEX_OUTOFBOUNDS_ERROR) {
			out.push_back(uc);
			uc = ucnv_getNextUChar(mGb18030Cnv, &in, (const char*)data + sz, &ec);
		}
		if (ep) *ep = size_t(in - (const char*)data);
	}
	return out;
}
//
std::string CodecGB18030::TranscodeToUTF8(const unsigned char* data, size_t sz, size_t* ep)
{
	std::string out;
	if (ep) *ep = sz;
	if (mGb18030Cnv) {
		UErrorCode ec = U_ZERO_ERROR;
		const char* in = (const char*)data;
		UChar32 uc = ucnv_getNextUChar(mGb18030Cnv, &in, in + sz, &ec);
		while (ec != U_INDEX_OUTOFBOUNDS_ERROR) {
			// utf32 -> utf8
			out += UTF32ToUTF8_(uc);
			// next
			uc = ucnv_getNextUChar(mGb18030Cnv, &in, in + sz, &ec);
		}
		if (ep) *ep = size_t(in - (const char*)data);
	}
	return out;
}
//
std::string CodecGB18030::TranscodeFromUTF8(const unsigned char* data, size_t sz, size_t* ep)
{
	std::string out;
	if (ep) *ep = sz;
	if (mUtf8Cnv) {
		std::vector<UChar> uchars;
		UErrorCode ec = U_ZERO_ERROR;
		const char* in = (const char*)data;
		UChar32 uc = ucnv_getNextUChar(mUtf8Cnv, &in, (const char*)data + sz, &ec);
		while (ec != U_INDEX_OUTOFBOUNDS_ERROR) {
			// utf32 -> utf16
			if (uc <= 0xffff) {
                uchars.push_back(uc);
            }
            else {
                // Surrogate
                uint16_t h = 0xd800 | ((uc & 0x000ffc00) >> 10);
                uint16_t l = 0xdc00 | ((uc & 0x3ff));
                uchars.push_back(h);
                uchars.push_back(l);
            }
			// next
			uc = ucnv_getNextUChar(mUtf8Cnv, &in, (const char*)data + sz, &ec);
		}
		if (ep) *ep = size_t(in - (const char*)data);
		// utf16 -> 18030
		if (mGb18030Cnv) {
			ec = U_ZERO_ERROR;
		    int32_t cnt = ucnv_fromUChars(mGb18030Cnv, 0, 0, uchars.data(), (int32_t)uchars.size(), &ec);
		    std::vector<char> tmp((size_t)cnt+1);
		    ec = U_ZERO_ERROR;
		    ucnv_fromUChars(mGb18030Cnv, tmp.data(), (int32_t)tmp.size(), uchars.data(), (int32_t)uchars.size(), &ec);
		    out.insert(out.end(), tmp.begin(), tmp.begin() + cnt);
		}
	}
	return out;
}
//
std::string CodecGB18030::TranscodeFromUTF32(const uint32_t* data, size_t sz)
{
	std::string out;
	// utf16 -> 18030
	if (mGb18030Cnv) {
        std::vector<UChar> uchars;
	    for (size_t k = 0; k < sz; ++k) {
	        if (data[k] <= 0xffff) {
	            uchars.push_back(data[k]);
	        } else {
	            uint16_t h = 0xd800 | ((data[k] & 0x000ffc00) >> 10);
                uint16_t l = 0xdc00 | ((data[k] & 0x3ff));
                uchars.push_back(h);
                uchars.push_back(l);
	        }
		}
		UErrorCode ec = U_ZERO_ERROR;
	    int32_t cnt = ucnv_fromUChars(mGb18030Cnv, 0, 0, uchars.data(), (int32_t)uchars.size(), &ec);
	    std::vector<char> tmp((size_t)cnt+1);
	    ec = U_ZERO_ERROR;
	    ucnv_fromUChars(mGb18030Cnv, tmp.data(), (int32_t)tmp.size(), uchars.data(), (int32_t)uchars.size(), &ec);
	    out.insert(out.end(), tmp.begin(), tmp.begin() + cnt);
	}
	return out;
}

#endif
#endif
#endif

#endif
