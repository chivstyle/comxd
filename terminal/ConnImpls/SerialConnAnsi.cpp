//
// (c) 2020 chiv
//
#include "SerialConnAnsi.h"
#include "AnsiControlSeq.h"

using namespace Upp;

SerialConnAnsi::SerialConnAnsi(std::shared_ptr<SerialIo> serial)
	: SerialConnVT(serial)
	, SerialConnECMA48(serial)
{
	InstallAnsiFunctions();
	mCursorXy.x = 0;
	mCursorXy.y = 0;
}

void SerialConnAnsi::InstallAnsiFunctions()
{
	mAnsiTrivialHandlers["[?25h"] = [=]() { mShowCursor = true; };
	mAnsiTrivialHandlers["[?25l"] = [=]() { mShowCursor = false; };
	mAnsiTrivialHandlers["[s"] = [=]() {
		mCursorXy = Point(mVx, mVy);
	};
	mAnsiTrivialHandlers["[u"] = [=]() {
		mVx = mCursorXy.x;
		mVy = mCursorXy.y;
		UpdatePresentationPos();
	};
}

void SerialConnAnsi::ProcessAnsiTrivial(const std::string& seq)
{
    auto it = mAnsiTrivialHandlers.find(seq);
    if (it != mAnsiTrivialHandlers.end()) {
        it->second();
    }
}

int SerialConnAnsi::IsControlSeq(const std::string& seq)
{
    auto seq_type = IsAnsiControlSeq(seq);
    if (seq_type == SEQ_NONE) {
        seq_type = SerialConnECMA48::IsControlSeq(seq);
        if (seq_type == SEQ_NONE) {
            seq_type = SerialConnVT::IsControlSeq(seq);
        }
    }
    return seq_type;
}

bool SerialConnAnsi::ProcessControlSeq(const std::string& seq, int seq_type)
{
    if (seq_type == Ansi_Trivial) {
        ProcessAnsiTrivial(seq);
    } else {
        bool processed = SerialConnECMA48::ProcessControlSeq(seq, seq_type);
        if (!processed) {
            return SerialConnVT::ProcessControlSeq(seq, seq_type);
        }
    }
    return true;
}
