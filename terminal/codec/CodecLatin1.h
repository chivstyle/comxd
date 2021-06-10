//
// (c) 2020 chiv
//
#pragma once

#include "Codec.h"

class CodecLatin1 : public Codec {
public:
    std::vector<uint32_t> TranscodeToUTF32(const unsigned char* data, size_t sz, size_t& ep);
    std::string TranscodeToUTF8(const unsigned char* data, size_t sz);
    std::string TranscodeFromUTF32(const uint32_t* data, size_t sz);
    std::string TranscodeFromUTF8(const unsigned char* data, size_t sz);
};
