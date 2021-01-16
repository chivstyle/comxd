//
// (C) 2020 chiv
//
#ifndef _comxd_ControlSeq_h_
#define _comxd_ControlSeq_h_

#include <list>
#include <string>
//----------------------------------------------------------------------------------------------
// CONTROl SEQ
#define SEQ_NONE                      0
#define SEQ_PENDING                   1
#define SEQ_CORRUPTED                 2
// ECMA-048
#define ECMA48_SEQ_BEGIN              3
#define ECMA48_SEQ_MAX_COUNT          200
#define ECMA48_SEQ_END                ECMA48_SEQ_BEGIN+ECMA48_SEQ_MAX_COUNT // 203
// vt102
#define VT102_SEQ_BEGIN               ECMA48_SEQ_END+1 // 204
#define VT102_SEQ_MAX_COUNT           100
#define VT102_SEQ_END                 VT102_SEQ_BEGIN+VT102_SEQ_MAX_COUNT // 304
//
struct ControlSeq {
    int         Type;
    std::string Head; // Begin with this string.
    std::string Tail; // End
    short       Pnum;   // Number of parameters
    enum ParaType {
        No,            // Withoud parameter
        Ps,            // With Ps
        Pn,            // With Pn
        Gs             // With string
    };
    short       Ptyp;   // Type of parameters
    //
    ControlSeq()
        : Type(SEQ_NONE)
        , Ptyp(No)
        , Pnum(0)
    {
    }
    ControlSeq(int type, const std::string& head, int ptyp, int pnum, const std::string& tail)
        : Type(type)
        , Head(head)
        , Tail(tail)
        , Pnum(pnum)
        , Ptyp(ptyp)
    {
    }
};

class ControlSeqFactory {
public:
    ControlSeqFactory() {}
    virtual ~ControlSeqFactory() {}
    /// @return
    ///     SEQ_NONE
    ///     SEQ_PENDING
    ///     Valid Sequence Type
    int IsControlSeq(const std::string& seq, size_t& p_begin, size_t& p_sz);
    //
    void Add(int type, const std::string& head, int ptyp, int pnum, const std::string& tail)
    {
        mSeqs.emplace_back(type, head, ptyp, pnum, tail);
    }
protected:
    std::list<ControlSeq> mSeqs;
};

#define REGISTER_SEQ(factory, type, head, ptyp, pnum, tail) \
    factory->Add(type, head, ControlSeq::ptyp, pnum, tail)

#endif
