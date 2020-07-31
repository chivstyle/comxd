//
// (c) 2020 chiv
//
#ifndef _comxd_ECMA48ControlSeq_h_
#define _comxd_ECMA48ControlSeq_h_

static const char* kECMA48CtrlSeqs[] = {
};
static inline int IsECMA48TrivialSeq(const std::string& seq)
{
    size_t vsz = sizeof(kECMA48CtrlSeqs) / sizeof(kECMA48CtrlSeqs[0]);
    size_t seq_sz = seq.length();
    for (size_t k = 0; k < vsz; ++k) {
        size_t pat_sz = strlen(kECMA48CtrlSeqs[k]);
        if (seq_sz > pat_sz) continue; // try next
        if (strncmp(kECMA48CtrlSeqs[k], seq.c_str(), seq_sz) == 0) {
            if (seq_sz < pat_sz) return 1; // maybe, you should give more chars
            else if (seq_sz == pat_sz) return 2; // It's a ECMA48 control seq
        }
    }
    return 0;
}
// match the control seq
// return 0 - It's not a ECMA48 control seq absolutely
//        1 - maybe, your should give more characters
//        2 - It's a ECMA48 control seq.
static inline int IsECMA48ControlSeq(const std::string& seq)
{
    std::function<int(const std::string&)> funcs[] = {
        IsECMA48TrivialSeq
    };
    int ret = 0;
    for (int i = 0; i < sizeof(funcs) / sizeof(funcs[0]); ++i) {
        ret = funcs[i](seq);
        if (ret == 0) continue; else break;
    }
    return ret;
}

#endif
