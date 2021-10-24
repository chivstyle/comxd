//
// (c) 2020 chiv
//
#pragma once

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#define LAYOUTFILE <comxd/comxd.lay>
#include <CtrlCore/lay.h>

#define TFILE <comxd/comxd.t>
#include <Core/t.h>

#define IMAGECLASS comxd
#define IMAGEFILE <comxd/comxd.iml>
#include <Draw/iml_header.h>

#define TOPICFILE <comxd/comxd.tpp/all.i>
#include <Core/topic_group.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))
#endif
