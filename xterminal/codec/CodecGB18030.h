//
// (c) 2020 chiv
//
#pragma once

#include "Codec.h"

#ifndef WITHOUT_ICU

struct UConverter;
class CodecGB18030 : public Codec {
public:
	CodecGB18030();
	~CodecGB18030();
	//
    std::vector<uint32_t> TranscodeToUTF32(const unsigned char* data, size_t sz, size_t* ep);
    std::string TranscodeToUTF8(const unsigned char* data, size_t sz, size_t* ep);
    std::string TranscodeFromUTF32(const uint32_t* data, size_t sz);
    std::string TranscodeFromUTF8(const unsigned char* data, size_t sz, size_t* ep);
private:
    UConverter* mUtf8Cnv;
    UConverter* mGb18030Cnv;
};

#endif
