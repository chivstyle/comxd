//
// (c) 2020 chiv
//
#pragma once

#include "ControlSeq.h"
#include <string>
#include <functional>

static const char* kAnsiCtrlSeqs[] = {
	"[?25h", // show cursor
	"[?25l", // hide cursor
	//-----------------------------------------------
	// Set Mode
	"[=0h",  // 40x25 monochrome (text)
	"[=1h",  // 40x25 color (text)
	"[=2h",  // 80x25 monochrome (text)
	"[=3h",  // 80x25 color (text)
	"[=4h",  // 320x200 4-color (graphics)
	"[=5h",  // 320x200 monochrome (graphics)
	"[=6h",  // 640x200 monochrome (graphics)
	"[=7h",  // Enables line wrapping
	"[=13h",  // 320x200 color (graphics)
	"[=14h",  // 640x200 color (16-color graphics)
	"[=15h",  // 640x350 monochrome (2-color graphics)
	"[=16h",  // 640x350 color (16-color graphics)
	"[=17h",  // 640x480 monochrome (2-color graphics)
	"[=18h",  // 640x480 color (16-color graphics)
	"[=19h",  // 320x200 color (256-color graphics)
	// Reset Mode
	"[=0l",  // 40x25 monochrome (text)
	"[=1l",  // 40x25 color (text)
	"[=2l",  // 80x25 monochrome (text)
	"[=3l",  // 80x25 color (text)
	"[=4l",  // 320x200 4-color (graphics)
	"[=5l",  // 320x200 monochrome (graphics)
	"[=6l",  // 640x200 monochrome (graphics)
	"[=7l",  // Enables line wrapping
	"[=13l",  // 320x200 color (graphics)
	"[=14l",  // 640x200 color (16-color graphics)
	"[=15l",  // 640x350 monochrome (2-color graphics)
	"[=16l",  // 640x350 color (16-color graphics)
	"[=17l",  // 640x480 monochrome (2-color graphics)
	"[=18l",  // 640x480 color (16-color graphics)
	"[=19l",  // 320x200 color (256-color graphics)
	//
	"[s", // save cursor position
	"[u"  // restore cursor position
};

enum AnsiSeqType {
	Ansi_Trivial = ANSI_SEQ_BEGIN,
	Ansi_KeyboardString,
	Ansi_SeqType_Endup
};

static inline int IsAnsiKeyboardString(const std::string& seq)
{
	size_t seq_sz = seq.length();
    if (seq_sz == 1 && seq[0] == '[')
        return SEQ_PENDING; // pending
    else {
        if (seq[0] == '[') {
            size_t b = 1;
            while (b < seq_sz) {
                if (seq[b] >= '0' && seq[b] <= '9' || seq[b] == ';')
                    b++;
                else
                    break;
            }
            if (b == seq_sz)
                return SEQ_PENDING;
            else {
                if (seq[b] == '\"') { // A string
                    if (seq_sz - b > 2 && seq[seq_sz-2] == '\"' && seq[seq_sz-1] == 'p')
                        return Ansi_KeyboardString;
                    return SEQ_PENDING;
                }
            }
        }
    }
    return SEQ_NONE;
}

static inline int IsAnsiTrivialSeq(const std::string& seq)
{
    const size_t vsz = sizeof(kAnsiCtrlSeqs) / sizeof(kAnsiCtrlSeqs[0]);
    size_t seq_sz = seq.length();
    for (size_t k = 0; k < vsz; ++k) {
        size_t pat_sz = strlen(kAnsiCtrlSeqs[k]);
        if (seq_sz > pat_sz) continue; // try next
        if (strncmp(kAnsiCtrlSeqs[k], seq.c_str(), seq_sz) == 0) {
            if (seq_sz < pat_sz) return SEQ_PENDING; // maybe, you should give more chars
            else if (seq_sz == pat_sz) return Ansi_Trivial; // Trivial VT102 seq
        }
    }
    return SEQ_NONE;
}

static inline int IsAnsiControlSeq(const std::string& seq)
{
    std::function<int(const std::string&)> funcs[] = {
        IsAnsiTrivialSeq,
        IsAnsiKeyboardString
    };
    int ret = SEQ_NONE;
    for (int i = 0; i < sizeof(funcs) / sizeof(funcs[0]); ++i) {
        ret = funcs[i](seq);
        if (ret == SEQ_NONE) continue;else break;
    }
    return ret;
}