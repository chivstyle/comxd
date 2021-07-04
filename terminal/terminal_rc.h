//
// (c) 2020 chiv
//
#ifndef _terminal_terminal_rc_h_
#define _terminal_terminal_rc_h_

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#define LAYOUTFILE <terminal/terminal.lay>
#include <CtrlCore/lay.h>

#define TFILE <terminal/terminal.t>
#include <Core/t.h>

#define IMAGECLASS terminal
#define IMAGEFILE <terminal/terminal.iml>
#include <Draw/iml_header.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

#endif
