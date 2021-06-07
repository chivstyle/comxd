/*!
// (c) 2021 chiv
//
*/
#pragma once

#define CS_DEFAULT               0

#define VT100_CS_BEGIN           1
#define VT100_CS_COUNT           10
#define VT100_CS_END             VT100_CS_BEGIN+VT100_CS_COUNT

#define ANSI_CS_BEGIN            VT100_CS_END+1
#define ANSI_CS_COUNT            10
#define ANSI_CS_END              ANSI_CS_BEGIN+ANSI_CS_COUNT

#define VT200_CS_BEGIN           ANSI_CS_END+1
#define VT200_CS_COUNT           20
#define VT200_CS_END             VT200_CS_BEGIN+VT200_CS_COUNT

#define VT300_CS_BEGIN           VT200_CS_END+1
#define VT300_CS_COUNT           10
#define VT300_CS_END             VT300_CS_BEGIN+VT300_CS_COUNT

#define VT400_CS_BEGIN           VT300_CS_END+1
#define VT400_CS_COUNT           10
#define VT400_CS_END             VT400_CS_BEGIN+VT400_CS_COUNT

#define VT500_CS_BEGIN           VT400_CS_END+1
#define VT500_CS_COUNT           10
#define VT500_CS_END             VT500_CS_BEGIN+VT500_CS_COUNT

#define XTERM_CS_BEGIN           VT500_CS_END+1
#define XTERM_CS_COUNT           10
#define XTERM_CS_END             XTERM_CS_BEGIN+XTERM_CS_COUNT
