#ifndef _terminal_vtwchar_h_
#define _terminal_vtwchar_h_

#ifdef __LINUX__
#define _XOPEN_SOURCE
#include <wchar.h>
#else
#include <stdint.h>
int wcwidth(uint32_t ucs);
#endif

#endif
