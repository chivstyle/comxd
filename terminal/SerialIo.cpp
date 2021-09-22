//
// (c) 2020 chiv
//
#include "SerialIo.h"

SerialIo::SerialIo()
{
}

SerialIo::~SerialIo()
{
}

std::vector<unsigned char> SerialIo::ReadRaw(size_t sz)
{
    std::vector<unsigned char> buf(sz);
    Read(buf.data(), sz);
    return buf;
}

std::string SerialIo::Read(size_t sz)
{
    char* buf = new char[sz + 1];
    Read((unsigned char*)buf, sz);
    buf[sz] = '\0';
    std::string out(buf);
    delete[] buf;
    return out;
}

size_t SerialIo::Write(const std::string& d)
{
    return Write((const unsigned char*)d.c_str(), d.length());
}

size_t SerialIo::Write(const std::vector<unsigned char>& d)
{
    return Write(d.data(), d.size());
}
