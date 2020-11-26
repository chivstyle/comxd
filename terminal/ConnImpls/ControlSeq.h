//
// (C) 2020 chiv
//
#ifndef _comxd_ControlSeq_h_
#define _comxd_ControlSeq_h_

//----------------------------------------------------------------------------------------------
// CONTROl SEQ
#define SEQ_NONE                      0
#define SEQ_PENDING                   1
// VT102 SPAN
#define VT102_SEQ_BEGIN               2
#define VT102_SEQ_COUNT               200
#define VT102_SEQ_END                 VT102_SEQ_BEGIN+VT102_SEQ_COUNT
// ECMA48
#define ECMA48_SEQ_BEGIN              VT102_SEQ_END+1
#define ECMA48_SEQ_COUNT              200
#define ECMA48_SEQ_END                ECMA48_SEQ_BEGIN+ECMA48_SEQ_COUNT
// Xterm
#define XTERM_SEQ_BEGIN               ECMA48_SEQ_END+1
#define XTERM_SEQ_COUNT               200
#define XTERM_SEQ_END                 XTERM_SEQ_BEGIN+XTERM_SEQ_COUNT
// Ansi
#define ANSI_SEQ_BEGIN                XTERM_SEQ_END+1
#define ANSI_SEQ_COUNT                200
#define ANSI_SEQ_END                  ANSI_SEQ_BEGIN+ANSI_SEQ_COUNT
//----------------------------------------------------------------------------------------------
// VT220
#define VT220_SEQ_BEGIN               ANSI_SEQ_END+1
#define VT220_SEQ_COUNT               20
#define VT220_SEQ_END                 VT220_SEQ_BEGIN+VT220_SEQ_COUNT

#endif
