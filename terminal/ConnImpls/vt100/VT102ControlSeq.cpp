//
// (c) 2021 chiv
//
#include "VT102ControlSeq.h"

static const char* kESC = "\x1b";
static const char* kCSI = "\x1bx5b";
//
static const char* kAPC = "\x1bx5f";
static const char* kCMD = "\x1bx64";
static const char* kDCS = "\x1bx50";
static const char* kOSC = "\x1bx5d";
static const char* kPM  = "\x1bx5e";
static const char* kSOS = "\x1bx58";
static const char* kST  = "\x1bx5c";

void AddVT102ControlSeqs(ControlSeqFactory* factory)
{
    REGISTER_SEQ(factory, ECMA48_APC, kCSI, Gs, 1, kST);
    REGISTER_SEQ(factory, ECMA48_BPH, kESC, No, 0, "\x42");
    REGISTER_SEQ(factory, ECMA48_CBT, kCSI, Pn, 1, "\x5a");
}
