//
// (c) 2020 chiv
//
#pragma once

#include "ControlSeq.h"
#include <string>
#include <functional>

static const char* kVT510CtrlSeqs[] = {
	"[?0J",
	"[?1J",
	"[?2J",
	"[?0K",
	"[?1K",
	"[?2K",
};

enum VT510SeqType {
	VT510_Trivial = ANSI_SEQ_BEGIN,
	VT510_SeqType_Endup
};

static inline int IsVT510TrivialSeq(const std::string& seq)
{
    const size_t vsz = sizeof(kVT510CtrlSeqs) / sizeof(kVT510CtrlSeqs[0]);
    size_t seq_sz = seq.length();
    for (size_t k = 0; k < vsz; ++k) {
        size_t pat_sz = strlen(kVT510CtrlSeqs[k]);
        if (seq_sz > pat_sz) continue; // try next
        if (strncmp(kVT510CtrlSeqs[k], seq.c_str(), seq_sz) == 0) {
            if (seq_sz < pat_sz) return SEQ_PENDING; // maybe, you should give more chars
            else if (seq_sz == pat_sz) return VT510_Trivial; // Trivial VT102 seq
        }
    }
    return SEQ_NONE;
}

static inline int IsVT510ControlSeq(const std::string& seq)
{
    std::function<int(const std::string&)> funcs[] = {
        IsVT510TrivialSeq
    };
    int ret = SEQ_NONE;
    for (int i = 0; i < sizeof(funcs) / sizeof(funcs[0]); ++i) {
        ret = funcs[i](seq);
        if (ret == SEQ_NONE) continue;else break;
    }
    return ret;
}