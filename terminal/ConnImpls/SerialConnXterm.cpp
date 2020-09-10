//
// (c) 2020 chiv
//
#include "SerialConnXterm.h"
#include "XtermControlSeq.h"
#include "ConnFactory.h"


REGISTER_CONN_INSTANCE("Xterm", SerialConnXterm);

using namespace Upp;

SerialConnXterm::SerialConnXterm(std::shared_ptr<SerialIo> serial)
    : Superclass(serial)
    , mIsAltScr(false)
{
    InstallXtermFunctions();
    // The default setting of alt screen is fixed, inherit from VT.
    // The user can change settings when the alt screen is active, but we do not store the
    // configuration after the user exit the alt screen.
    SaveScr(mAltScr);
}
//
SerialConnXterm::~SerialConnXterm()
{
}

void SerialConnXterm::SaveScr(ScreenData& sd)
{
    sd.Vx = mVx;
    sd.Vy = mVy;
    sd.Font = mFont;
    sd.Blink = mBlink;
    sd.BgColor = mBgColor;
    sd.FgColor = mFgColor;
    sd.AttrFuncs = mCurrAttrFuncs;
    sd.LinesBuffer = mLinesBuffer;
    sd.Lines = mLines;
}

void SerialConnXterm::LoadScr(const ScreenData& sd)
{
    mLinesBuffer = sd.LinesBuffer;
    mLines = sd.Lines;
    mCurrAttrFuncs = sd.AttrFuncs;
    mVx = sd.Vx;
    mVy = sd.Vy;
    mFgColor = sd.FgColor;
    mBgColor = sd.BgColor;
    mBlink = sd.Blink;
    mFont = sd.Font;
    mFontW = mFont.GetAveWidth();
    mFontH = mFont.GetLineHeight();
    //
    DoLayout();
    UpdatePresentation();
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
    int ret = Superclass::IsControlSeq(seq);
    if (ret == 0) {
        return IsXtermControlSeq(seq);
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
        GetSerial()->Write(d);
        return true;
    }
    return Superclass::ProcessKeyDown(key, flags);
}

void SerialConnXterm::InstallXtermFunctions()
{
    mXtermTrivialHandlers["[?1049h"] = [=]() { // switch to alternative screen
        if (mIsAltScr == false) {
            SaveScr(mBkgScr); // backup current screen
            LoadScr(mAltScr); // load alternative screen.
            mIsAltScr = true;
        }
    };
    mXtermTrivialHandlers["[?1049l"] = [=]() { // switch to main screen
        if (mIsAltScr) {
            LoadScr(mBkgScr);
            mIsAltScr = false;
        }
    };
}
