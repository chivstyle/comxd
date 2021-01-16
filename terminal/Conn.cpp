//
// (c) 2020 chiv
//
#include "Conn.h"
#include "codec/CodecUTF8.h"
#include "CodecFactory.h"

SerialConn::SerialConn(std::shared_ptr<SerialIo> io)
    : mIo(io)
{
    // default: UTF-8
    mCodec = std::make_shared<CodecUTF8>();
}

void SerialConn::SetCodec(const char* codec_name)
{
    mCodec = std::shared_ptr<Codec>(CodecFactory::Inst()->CreateInst(codec_name));
}
