//
// (c) 2020 chiv
//
#include "ControlSeq.h"
/// return - <0 Failed
///        - =0 Need more
///        - >0 End of pn
static inline int ParsePs(const std::string_view& seq, size_t p_begin)
{
    while (seq[p_begin] >= '0' && seq[p_begin] <= '9') {
        p_begin++;
    }
    if (p_begin == seq.length())
        return 0; // Need more
    else {
        return (int)p_begin;
    }
}
//
static inline int ParsePs(const std::string_view& seq, size_t p_begin, int pn_count)
{
    if (pn_count > 0) {
        int p = ParsePs(seq, p_begin);
        if (p == 0)
            return 0;
        else {
            pn_count--;
            while (pn_count--) {
                if (seq[p] == ';') {
                    p++;
                    if (p >= seq.length())
                        return 0; // need more
                    //
                    p = ParsePs(seq, p);
                    if (p == 0)
                        return 0;
                } else
                    return -1;
            }
            return p;
        }
    } else {
        while (seq[p_begin] >= '0' && seq[p_begin] <= '9' || seq[p_begin] == ';') {
            p_begin++;
        }
        if (p_begin == seq.length())
            return 0; // Need more
        else {
            return (int)p_begin;
        }
    }
}
/// return - <0 Failed
///        - =0 Need more
///        - >0 End of pn
static inline int ParsePn(const std::string_view& seq, size_t p_begin)
{
    while (seq[p_begin] >= '0' && seq[p_begin] <= '9') {
        p_begin++;
    }
    if (p_begin == seq.length())
        return 0; // Need more
    else {
        return (int)p_begin;
    }
}

static inline int ParsePn(const std::string_view& seq, size_t p_begin, int pn_count)
{
    if (pn_count > 0) {
        int p = ParsePn(seq, p_begin);
        if (p == 0)
            return 0;
        else {
            pn_count--;
            while (pn_count--) {
                if (seq[p] == ';') {
                    p++;
                    if (p >= seq.length())
                        return 0; // need more
                    //
                    p = ParsePn(seq, p);
                    if (p == 0)
                        return 0;
                } else
                    return -1;
            }
            return p;
        }
    } else {
        while (seq[p_begin] >= '0' && seq[p_begin] <= '9' || seq[p_begin] == ';') {
            p_begin++;
        }
        if (p_begin == seq.length())
            return 0; // Need more
        else {
            return (int)p_begin;
        }
    }
}
// Valid chars, from 0x20~0x74
static inline int ParseGs(const std::string_view& seq, size_t p_begin)
{
    // we should accept anything except the ASCII control chars
    while ((unsigned char)seq[p_begin] >= 0x20 && seq[p_begin] != 0x7f) {
        p_begin++;
    }
    return (int)p_begin;
}
//
static inline int ParseGn(const std::string_view& seq, size_t p_begin, int pn)
{
    int cn = 0;
    for (size_t k = p_begin; k < seq.length() && cn < pn; ++k) {
        cn++;
    }
    return cn < pn ? 0 : (int)p_begin + cn;
}

int ControlSeqFactory::IsControlSeq(const std::string_view& seq, size_t& p_begin, size_t& p_sz, size_t& s_end)
{
    int type = SEQ_NONE;
    s_end = seq.length();
    size_t head_max = 0;
    for (auto it = mSeqs.begin(); it != mSeqs.end(); ++it) {
        // match head.
        if (head_max > 0 && it->Head.length() < head_max)
            continue;
        size_t k = 0;
        for (; k < seq.length() && k < it->Head.length(); ++k) {
            if (seq[k] != it->Head[k])
                break;
        }
        if (k == seq.length() && k < it->Head.length()) {
            type = SEQ_PENDING; // Need more chars
            break;
        } else if (k < it->Head.length()) {
            type = SEQ_NONE;
            continue;
        } // Not this, try next.
        if (k >= head_max)
            head_max = k;
        else
            continue;
        // save p_begin, parameter begins here
        p_begin = k;
        s_end = p_begin;
        // match tail.
        int ret = -1; // 0 - Need more, >0 the end of parameter, <0 parameter was wrong
        switch (it->Ptyp) {
        case ControlSeq::Pn:
            ret = ParsePn(seq, p_begin, it->Pnum);
            break;
        case ControlSeq::Ps:
            ret = ParsePn(seq, p_begin, it->Pnum);
            break;
        case ControlSeq::Gs:
            ret = ParseGs(seq, p_begin);
            break;
        case ControlSeq::Gn:
            ret = ParseGn(seq, p_begin, it->Pnum);
            break;
        case ControlSeq::No:
            ret = (int)p_begin;
            break;
        default:
            abort();
            break;
        }
        if (ret < 0)
            continue; // It's not a valid parameter, continue to try
        else if (ret == 0)
            return SEQ_PENDING;
        else {
            if (ret >= (int)seq.length() && !it->Tail.empty())
                return SEQ_PENDING;
            size_t ln = std::min(seq.length() - (size_t)ret, it->Tail.length()), i = 0;
            for (; i < ln; ++i) {
                if (seq[ret + i] != it->Tail[i])
                    break;
            }
            if (i == it->Tail.length()) {
                type = it->Type;
                p_sz = ret - p_begin;
                s_end = ret + i;
                break;
            } else if (i == ln && i < it->Tail.length()) {
                type = SEQ_PENDING;
                break;
            }
            s_end = ret + i;
        }
    }
    return type;
}
