//
// (c) 2020 chiv
//
#include "SerialConnVT510.h"
#include "VT510ControlSeq.h"

using namespace Upp;

SerialConnVT510::SerialConnVT510(std::shared_ptr<SerialIo> serial)
	: SerialConnVT(serial)
	, SerialConnVT102(serial)
{
	InstallVT510Functions();
}

void SerialConnVT510::InstallVT510Functions()
{
}

void SerialConnVT510::ProcessVT510Trivial(const std::string& seq)
{
    auto it = mVT510TrivialHandlers.find(seq);
    if (it != mVT510TrivialHandlers.end()) {
        it->second();
    }
}

int SerialConnVT510::IsControlSeq(const std::string& seq)
{
    auto seq_type = IsVT510ControlSeq(seq);
    if (seq_type == SEQ_NONE) {
        seq_type = SerialConnVT102::IsControlSeq(seq);
        if (seq_type == SEQ_NONE) {
            seq_type = SerialConnVT::IsControlSeq(seq);
        }
    }
    return seq_type;
}

bool SerialConnVT510::ProcessControlSeq(const std::string& seq, int seq_type)
{
    if (seq_type == VT510_Trivial) {
        ProcessVT510Trivial(seq);
    } else {
        bool processed = SerialConnVT102::ProcessControlSeq(seq, seq_type);
        if (!processed) {
            return SerialConnVT::ProcessControlSeq(seq, seq_type);
        }
    }
    return true;
}
