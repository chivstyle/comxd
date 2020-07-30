#ifndef _comxd_ControlSeq_h_
#define _comxd_ControlSeq_h_
//
enum SeqType {
    // 0,1,2, are reserved
    VT52_Cursor = 3,
    VT102_Cursor,
    VT102_EditingFunctions,
    VT102_ScrollingRegion
};

#endif
