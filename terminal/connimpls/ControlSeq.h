//
// (C) 2020 chiv
//
#ifndef _comxd_ControlSeq_h_
#define _comxd_ControlSeq_h_

#include <string>
#include <vector>
//----------------------------------------------------------------------------------------------
// CONTROl SEQ
#define SEQ_NONE 0
#define SEQ_PENDING 1
#define SEQ_CORRUPTED 2
// ECMA-048
#define ECMA48_SEQ_BEGIN 3
#define ECMA48_SEQ_MAX_COUNT 200
#define ECMA48_SEQ_END ECMA48_SEQ_BEGIN + ECMA48_SEQ_MAX_COUNT // 203
// vt100
#define VT100_SEQ_BEGIN ECMA48_SEQ_END + 1 // 204
#define VT100_SEQ_MAX_COUNT 100
#define VT100_SEQ_END VT100_SEQ_BEGIN + VT100_SEQ_MAX_COUNT // 304
// ansi
#define ANSI_SEQ_BEGIN VT100_SEQ_END + 1 // 305
#define ANSI_SEQ_MAX_COUNT 100
#define ANSI_SEQ_END ANSI_SEQ_BEGIN + ANSI_SEQ_MAX_COUNT // 405
// vt200
#define VT200_SEQ_BEGIN ANSI_SEQ_END + 1 // 406
#define VT200_SEQ_MAX_COUNT 100
#define VT200_SEQ_END VT200_SEQ_BEGIN + VT200_SEQ_MAX_COUNT // 506
// vt300
#define VT300_SEQ_BEGIN VT200_SEQ_END + 1 // 507
#define VT300_SEQ_MAX_COUNT 100
#define VT300_SEQ_END VT300_SEQ_BEGIN + VT300_SEQ_MAX_COUNT // 607
// vt400
#define VT400_SEQ_BEGIN VT300_SEQ_END + 1 // 608
#define VT400_SEQ_MAX_COUNT 100
#define VT400_SEQ_END VT400_SEQ_BEGIN + VT400_SEQ_MAX_COUNT // 708
// vt500
#define VT500_SEQ_BEGIN VT400_SEQ_END + 1 // 809
#define VT500_SEQ_MAX_COUNT 100
#define VT500_SEQ_END VT500_SEQ_BEGIN + VT500_SEQ_MAX_COUNT // 909
// xterm
#define XTERM_SEQ_BEGIN VT500_SEQ_END + 1 // 1010
#define XTERM_SEQ_MAX_COUNT 100
#define XTERM_SEQ_END XTERM_SEQ_BEGIN + XTERM_SEQ_MAX_COUNT // 1110
//
#define VTFLG_S8C  (0x1 << 16)
#define VTFLG_VTx  0xffff
enum VTOperatingLevel {
    VT100_S7C = 0x100,
    VT200_S7C = 0x200,
    VT200_S8C = VTFLG_S8C | VT200_S7C,
    VT300_S7C = 0x300,
    VT300_S8C = VTFLG_S8C | VT300_S7C,
    VT400_S7C = 0x400,
    VT400_S8C = VTFLG_S8C | VT400_S7C,
    VT500_S7C = 0x500,
    VT500_S8C = VTFLG_S8C | VT500_S7C
};
static inline int LevelToVTx(int level)
{
    return level & VTFLG_VTx;
}
//
struct ControlSeq {
    int Type;
    std::string Head; // Begin with this string.
    std::string Tail; // End
    short Pnum; // Number of parameters
    enum ParaType {
        No, // Withoud parameter
        Ps, // With Ps
        Pn, // With Pn
        Gs, // With string
        Gn // parameter is a string with length = n
    };
    short Ptyp; // Type of parameters
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
    ControlSeqFactory() { }
    virtual ~ControlSeqFactory() { }
    /// @return
    ///     SEQ_NONE
    ///     SEQ_PENDING
    ///     Valid Sequence Type
    int IsControlSeq(const std::string_view& seq, size_t& p_begin, size_t& p_sz, size_t& s_end);
    //
    void Add(int type, const std::string& head, int ptyp, int pnum, const std::string& tail)
    {
        mSeqs.emplace_back(type, head, ptyp, pnum, tail);
    }

protected:
    std::vector<ControlSeq> mSeqs;
};

#define REGISTER_SEQ(factory, type, head, ptyp, pnum, tail) \
    factory->Add(type, head, ControlSeq::ptyp, pnum, tail)

#endif
