//
// (c) 2020 chiv
//
#include "SerialConnXterm.h"
#include "XtermControlSeq.h"
#include "ConnFactory.h"


REGISTER_CONN_INSTANCE("Xterm", SerialConnXterm);

using namespace Upp;

SerialConnXterm::SerialConnXterm(std::shared_ptr<serial::Serial> serial)
    : Superclass(serial)
    , mIsAltScr(false)
{
    InstallXtermFunctions();
}
//
SerialConnXterm::~SerialConnXterm()
{
}

void SerialConnXterm::SaveScr(ScreenData& sd)
{
    SaveCursor(sd.CursorData_);
    sd.AttrFuncs_ = mCurrAttrFuncs;
    sd.LinesBuffer_ = mLinesBuffer;
    sd.Lines_ = mLines;
}

void SerialConnXterm::LoadScr(const ScreenData& sd)
{
    mLinesBuffer = sd.LinesBuffer_;
    mLines = sd.Lines_;
    mCurrAttrFuncs = sd.AttrFuncs_;
    // before LoadCursor, others should be loaded already.
    LoadCursor(sd.CursorData_);
}

void SerialConnXterm::ProcessXtermTrivial(const std::string& seq)
{
    auto it = mXtermTrivialHandlers.find(seq);
    if (it != mXtermTrivialHandlers.end()) {
        it->second();
    }
}

void SerialConnXterm::ProcessControlSeq(const std::string& seq, int seq_type)
{
    if (seq_type == Xterm_Trivial) {
        ProcessXtermTrivial(seq);
    } else if (seq_type > Xterm_Trivial && seq_type < Xterm_SeqType_Endup) {
        
    } else {
        Superclass::ProcessControlSeq(seq, seq_type);
    }
}

int SerialConnXterm::IsControlSeq(const std::string& seq)
{
    int ret = IsXtermControlSeq(seq);
    if (ret == 0) { // It's not a xterm control seq absolutely
        return Superclass::IsControlSeq(seq);
    }
    return ret;
}

bool SerialConnXterm::ProcessKeyDown(Upp::dword key, Upp::dword flags)
{
    std::string d;
    switch (key) {
    case K_END:
        d = "\033[F";
        break;
    }
    if (!d.empty()) {
        GetSerial()->write(d);
        return true;
    }
    return Superclass::ProcessKeyDown(key, flags);
}

void SerialConnXterm::InstallXtermFunctions()
{
    mXtermTrivialHandlers["[?1049h"] = [=]() { // switch to alternative screen
        SaveScr(mBkgScr); // backup current screen
        LoadScr(mAltScr); // load alternative screen.
        mIsAltScr = true;
    };
    mXtermTrivialHandlers["[?1049l"] = [=]() { // switch to main screen
        if (mIsAltScr) {
            LoadScr(mBkgScr);
            mIsAltScr = false;
        }
    };
}
