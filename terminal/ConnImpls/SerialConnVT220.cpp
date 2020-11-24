//
// (c) 2020 chiv
//
#include "SerialConnVT220.h"
#include "VT220ControlSeq.h"

using namespace Upp;

SerialConnVT220::SerialConnVT220(std::shared_ptr<SerialIo> serial)
	: SerialConnVT(serial)
	, SerialConnVT102(serial)
{
	InstallVT220Functions();
}

void SerialConnVT220::InstallVT220Functions()
{
}

void SerialConnVT220::ProcessVT220Trivial(const std::string& seq)
{
    auto it = mVT220TrivialHandlers.find(seq);
    if (it != mVT220TrivialHandlers.end()) {
        it->second();
    }
}

int SerialConnVT220::IsControlSeq(const std::string& seq)
{
    auto seq_type = IsVT220ControlSeq(seq);
    if (seq_type == SEQ_NONE) {
        seq_type = SerialConnVT102::IsControlSeq(seq);
        if (seq_type == SEQ_NONE) {
            seq_type = SerialConnVT::IsControlSeq(seq);
        }
    }
    return seq_type;
}

bool SerialConnVT220::ProcessControlSeq(const std::string& seq, int seq_type)
{
    if (seq_type == VT220_Trivial) {
        ProcessVT220Trivial(seq);
    } else {
        bool processed = SerialConnVT102::ProcessControlSeq(seq, seq_type);
        if (!processed) {
            return SerialConnVT::ProcessControlSeq(seq, seq_type);
        }
    }
    return true;
}
