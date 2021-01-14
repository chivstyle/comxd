//
// (c) 2020 chiv
//
#include "ControlSeq.h"
/// return - <0 Failed
///        - =0 Need more
///        - >0 End of pn
static inline int ParsePs(const std::string& seq, size_t p_begin)
{
    while (seq[p_begin] >= '0' && seq[p_begin] <= '9') {
        p_begin++;
    }
    if (p_begin == seq.length()) return 0; // Need more
    else {
        return (int)p_begin;
    }
}
//
static inline int ParsePs(const std::string& seq, size_t p_begin, int pn_count)
{
	if (pn_count > 0) {
	    int p = ParsePs(seq, p_begin);
	    if (p == 0) return 0;
	    else {
	        pn_count--;
	        while (pn_count--) {
	            if (seq[p] == ';') {
	                p++;
	                if (p >= seq.length()) return 0; // need more
	                //
	                p = ParsePs(seq, p);
	                if (p == 0) return 0;
	            } else return -1;
	        }
	        return p;
	    }
	} else {
		while (seq[p_begin] >= '0' && seq[p_begin] <= '9' || seq[p_begin] == ';') {
	        p_begin++;
	    }
	    if (p_begin == seq.length()) return 0; // Need more
	    else {
	        return (int)p_begin;
	    }
	}
}
/// return - <0 Failed
///        - =0 Need more
///        - >0 End of pn
static inline int ParsePn(const std::string& seq, size_t p_begin)
{
    while (seq[p_begin] >= '0' && seq[p_begin] <= '9') {
        p_begin++;
    }
    if (p_begin == seq.length()) return 0; // Need more
    else {
        return (int)p_begin;
    }
}

static inline int ParsePn(const std::string& seq, size_t p_begin, int pn_count)
{
	if (pn_count > 0) {
	    int p = ParsePn(seq, p_begin);
	    if (p == 0) return 0;
	    else {
	        pn_count--;
	        while (pn_count--) {
	            if (seq[p] == ';') {
	                p++;
	                if (p >= seq.length()) return 0; // need more
	                //
	                p = ParsePn(seq, p);
	                if (p == 0) return 0;
	            } else return -1;
	        }
	        return p;
	    }
	} else {
		while (seq[p_begin] >= '0' && seq[p_begin] <= '9' || seq[p_begin] == ';') {
	        p_begin++;
	    }
	    if (p_begin == seq.length()) return 0; // Need more
	    else {
	        return (int)p_begin;
	    }
	}
}
// Valid chars, from 0x20~0x74
static inline int ParseGs(const std::string& seq, size_t p_begin)
{
    while (seq[p_begin] >= 0x20 && seq[p_begin] < 0x7f) {
        p_begin++;
    }
    return (int)p_begin;
}

int ControlSeqFactory::IsControlSeq(const std::string& seq, size_t& p_begin, size_t& p_sz)
{
	if (*seq.rbegin() == ';' || *seq.rbegin() == ' ' ||
		*seq.rbegin() >= '0' && *seq.rbegin() <= '9') return SEQ_PENDING;
    int type = SEQ_NONE;
    int head_max = 0;
    for (auto it = mSeqs.begin(); it != mSeqs.end(); ++it) {
        // match head.
        if (head_max > 0 && it->Head.length() < head_max) continue;
        size_t k = 0;
        for (; k < seq.length() && k < it->Head.length(); ++k) {
            if (seq[k] != it->Head[k])
                break;
        }
        if (k == seq.length() && k < it->Head.length()) {
            type = SEQ_PENDING; // Need more chars
            break;
        } else if (k < it->Head.length()) { type = SEQ_NONE; continue; } // Not this, try next.
        if (k >= head_max) head_max = k;
        else continue;
        // p_begin
        p_begin = k;
        // pass parameters
        int ret = -1; // Not matched
        switch (it->Ptyp) {
        case ControlSeq::Pn: ret = ParsePn(seq, k, it->Pnum); break;
        case ControlSeq::Ps: ret = ParsePs(seq, k, it->Pnum); break;
        case ControlSeq::Gs: ret = ParseGs(seq, k); break;
        case ControlSeq::No: ret = (int)k; break;
        default:break;
        }
        if (ret < 0) { type = SEQ_NONE; continue; } // Bad parameters, try next
        if (!it->Tail.empty()) {
            if (ret == 0) {
                if (p_begin >= seq.length()-1) { type = SEQ_PENDING; break; }
                else {
                    type = SEQ_NONE;
                    continue;
                }
            }
            if (seq[ret] == '\r') { type = SEQ_UNKNOWN; break; } // CR will break any seq.
            if (it->Ptyp == ControlSeq::Gs && (seq[ret] < 0x20 || seq[ret] >= 0x7f)) { type = SEQ_UNKNOWN; break; }
	        // match tail
	        size_t ln = seq.length() - (size_t)ret; // Tail seq...

	        for (k = 0; k < ln && k < it->Tail.length(); ++k) {
	            if (seq[(size_t)ret+k] != it->Tail[k])
	                break;
	        }
	        if (k == ln &&  k < it->Tail.length()) {
	            type = SEQ_PENDING;
	            break;
	        } else if (k < it->Tail.length()) { type = SEQ_NONE;  continue; } // Not this, try next
	        k += (size_t)ret;
        }
        // p_sz
        p_sz = k - it->Tail.length() - p_begin;
        // reach here, assert
        type = it->Type;
        break;
    }
    return type;
}
