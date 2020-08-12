//
// (C) 2020 chiv
//
#ifndef _comxd_ControlSeq_h_
#define _comxd_ControlSeq_h_

#define SEQ_NONE                      0
#define SEQ_PENDING                   1
// VT102 SPAN
#define VT102_SEQ_BEGIN               2
#define VT102_SEQ_COUNT               200
#define VT102_SEQ_END                 VT102_SEQ_BEGIN+VT102_SEQ_COUNT
#define ECMA48_SEQ_BEGIN              VT102_SEQ_END+1
#define ECMA48_SEQ_COUNT              200
#define ECMA48_SEQ_END                ECMA48_SEQ_BEGIN+ECMA48_SEQ_COUNT


#endif
