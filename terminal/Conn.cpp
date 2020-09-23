//
// (c) 2020 chiv
//
#include "Conn.h"
#include "CodecFactory.h"

SerialConn::SerialConn(std::shared_ptr<SerialIo> serial)
    : mSerial(serial)
{
}

void SerialConn::SetCodec(const char* codec_name)
{
    mCodec = std::shared_ptr<Codec>(CodecFactory::Inst()->CreateInst(codec_name));
}
