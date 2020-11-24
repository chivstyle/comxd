//
// (c) 2020 chiv
//
#pragma once

#include "ControlSeq.h"
#include <string>
#include <functional>

static const char* kVT220CtrlSeqs[] = {
	"[?0J",
	"[?1J",
	"[?2J",
	"[?0K",
	"[?1K",
	"[?2K",
};

enum VT220SeqType {
	VT220_Trivial = ANSI_SEQ_BEGIN,
	VT220_SeqType_Endup
};

static inline int IsVT220TrivialSeq(const std::string& seq)
{
    const size_t vsz = sizeof(kVT220CtrlSeqs) / sizeof(kVT220CtrlSeqs[0]);
    size_t seq_sz = seq.length();
    for (size_t k = 0; k < vsz; ++k) {
        size_t pat_sz = strlen(kVT220CtrlSeqs[k]);
        if (seq_sz > pat_sz) continue; // try next
        if (strncmp(kVT220CtrlSeqs[k], seq.c_str(), seq_sz) == 0) {
            if (seq_sz < pat_sz) return SEQ_PENDING; // maybe, you should give more chars
            else if (seq_sz == pat_sz) return VT220_Trivial; // Trivial VT102 seq
        }
    }
    return SEQ_NONE;
}

static inline int IsVT220ControlSeq(const std::string& seq)
{
    std::function<int(const std::string&)> funcs[] = {
        IsVT220TrivialSeq
    };
    int ret = SEQ_NONE;
    for (int i = 0; i < sizeof(funcs) / sizeof(funcs[0]); ++i) {
        ret = funcs[i](seq);
        if (ret == SEQ_NONE) continue;else break;
    }
    return ret;
}