//
// (c) 2020 chiv
//
#include "SerialConnECMA48.h"
#include "ECMA48ControlSeq.h"
#include "ConnFactory.h"

// We implement this as the base-class of Xterm, we'll provide ecma48 in future.
// REGISTER_CONN_INSTANCE("ECMA-48", SerialConnECMA48);

using namespace Upp;

SerialConnECMA48::SerialConnECMA48(std::shared_ptr<serial::Serial> serial)
    : Superclass(serial)
{
    InstallEcma48Functions();
}
//
SerialConnECMA48::~SerialConnECMA48()
{
}

void SerialConnECMA48::ProcessVT102CharAttribute(int attr_code)
{
    switch (attr_code) {
    case 0:
        mCurrAttrFuncs.clear();
        mCurrAttrFuncs.push_back([=]() { SetDefaultStyle(); });
        break;
    case 1:
        mCurrAttrFuncs.push_back([=]() {
            mFont.Bold();
        });
        break;
    case 2:
        mCurrAttrFuncs.push_back([=]() {
            mFont.NoBold();
        });
        break;
    case 3:
        mCurrAttrFuncs.push_back([=]() {
            mFont.Italic();
        });
        break;
    case 4:
        mCurrAttrFuncs.push_back([=]() {
            mFont.Underline();
        });
        break;
    case 5:
    case 6: // rapidly blinking, No, we treat is as 5
        mCurrAttrFuncs.push_back([=]() {
            mBlink = true;
        });
        break;
    case 7:
        mCurrAttrFuncs.push_back([=]() {
            std::swap(mFgColor, mBgColor);
        });
        break;
    case 8: // conceal
        mCurrAttrFuncs.push_back([=]() {
            mVisible = false;
        });
        break;
    case 9: // crossed-out
        mCurrAttrFuncs.push_back([=]() {
            mFont.Strikeout();
        });
        break;
    case 10: // fonts
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20: // Fraktur
    case 21: // double underlined
    case 22: // normal color, normal intensity
        mCurrAttrFuncs.push_back([=]() {
            mFont.NoBold();
            mFgColor = mDefaultFgColor;
        });
        break;
    case 23: // Not italicized, not fraktur
        mCurrAttrFuncs.push_back([=]() {
            mFont.NoItalic();
        });
        break;
    case 24: // not underline
        mCurrAttrFuncs.push_back([=]() {
            mFont.NoUnderline();
        });
        break;
    case 25: // steady, no blinking
        mCurrAttrFuncs.push_back([=]() {
            mBlink = false;
        });
        break;
    case 26: // reserved for proportional spacing as specified in CCITT recommendation T6.1
    case 27: // positive image
        break;
    case 28: // revealed characters
        mCurrAttrFuncs.push_back([=]() {
            mVisible = true;
        });
        break;
    case 29: // not crossed
        mCurrAttrFuncs.push_back([=]() {
            mFont.NoStrikeout();
        });
        break;
    case 30: // black
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(0, 0, 0);
        });
        break;
    case 31: // red
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(255, 0, 0);
        });
        break;
    case 32: // green
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(0, 255, 0);
        });
        break;
    case 33: // yellow
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(255, 255, 0);
        });
        break;
    case 34: // blue
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(0, 0, 255);
        });
        break;
    case 35: // magenta
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(255, 0, 255);
        });
        break;
    case 36: // cyan
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(0, 255, 255);
        });
        break;
    case 37: // white
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(255, 255, 255);
        });
        break;
    case 38: // (reserved for future standardization; intended for setting character foreground colour as specified in
             // ISO 8613-6 [CCITT Recommendation T.416])
        break;
    case 39: // default color
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = mDefaultFgColor;
        });
        break;
    case 40: // black background
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(0, 0, 0);
        });
        break;
    case 41: // red bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(255, 0, 0);
        });
        break;
    case 42: // green bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(0, 255, 0);
        });
        break;
    case 43: // yellow bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(255, 255, 0);
        });
        break;
    case 44: // blue bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(0, 0, 255);
        });
        break;
    case 45: // magenta bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(255, 0, 255);
        });
        break;
    case 46: // cyan bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(0, 255, 255);
        });
        break;
    case 47: // white bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(255, 255, 255);
        });
        break;
    case 48: // (reserved for future standardization; intended for setting character foreground colour as specified in
             // ISO 8613-6 [CCITT Recommendation T.416])
        break;
    case 49: // default color bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = mDefaultBgColor;
        });
        break;
    case 50: // (reserved for cancelling the effect of the rendering aspect established by parameter value 26)
        break;
    case 51: // framed
    case 52: // encircled
    case 53: // overlined
    case 54: // not framed, not encircled
    case 55: // not overlined
        break;
    case 56: // reserved for future
    case 57:
    case 58:
    case 59:
        break;
    case 60: // ideogram underline or right side line
    case 61: // ideogram double underline or double line on the right side
    case 62: // ideogram overline or left side line
    case 63: // ideogram double overline or double line on the left side
    case 64: // ideogram stress marking
    case 65: // cancels the effect of the rendition aspects established by parameter values 60 to 64
        break;
    }
}

void SerialConnECMA48::ProcessSGR(const std::string& seq)
{
    // we override the ProcessVT102CharAttribute, the ProcessVT102CharAttributes will invoke it
    // when it splits a attr_code.
    // ECMA48 SGR, [Ps...]<m>, the pattern is compatible with VT102.
    Superclass::ProcessVT102CharAttributes(seq);
}

void SerialConnECMA48::InstallEcma48Functions()
{
    mEcma48Funcs[ECMA48_SGR] = [=](const std::string& seq) { SerialConnECMA48::ProcessSGR(seq); };
    // cursor movement, VT102 compatible
    mEcma48Funcs[ECMA48_CUB] = [=](const std::string& seq) { Superclass::ProcessVT102CursorMovementCmds(seq); };
    mEcma48Funcs[ECMA48_CUD] = mEcma48Funcs[ECMA48_CUF]
                             = mEcma48Funcs[ECMA48_CUP]
                             = mEcma48Funcs[ECMA48_CUU]
                             = mEcma48Funcs[ECMA48_CUB];
    // Erase in page, is compatible with VT102, VT102_Trivial could process it.
    mEcma48Funcs[ECMA48_ED] = [=](const std::string& seq) { Superclass::ProcessVT102Trivial(seq); };
    // TODO: support more ecma48 functions
    mEcma48Funcs[ECMA48_C1] = [=](const std::string& seq) { ProcessC1(seq[0]); };
}

void SerialConnECMA48::ProcessEcma48Trivial(const std::string& seq)
{
    auto it = mEcma48TrivialHandlers.find(seq);
    if (it != mEcma48TrivialHandlers.end()) {
        it->second(seq);
    }
}

int SerialConnECMA48::IsControlSeq(const std::string& seq)
{
    int ret = Superclass::IsControlSeq(seq);
    if (ret == 0) {
        ret = Superclass::IsControlSeq(seq);
    }
    return ret;
}

void SerialConnECMA48::ProcessControlSeq(const std::string& seq, int seq_type)
{
    // for faster, we forward the seq_type as accurately as we could
    if (seq_type == ECMA48_Trivial) ProcessEcma48Trivial(seq); else {
        if (seq_type > ECMA48_Trivial && seq_type < ECMA48_SeqType_Endup) {
            auto it = mEcma48Funcs.find(seq_type);
            if (it != mEcma48Funcs.end()) {
                it->second(seq);
            }
        } else {
            Superclass::ProcessControlSeq(seq, seq_type);
        }
    }
}

void SerialConnECMA48::ProcessC0(char cc)
{
    (void)cc;
}

void SerialConnECMA48::ProcessC1(char cc)
{
    // We do not support All C1 codes
    std::string seq; seq.push_back(cc);
    Superclass::ProcessVT102Trivial(seq);
}

void SerialConnECMA48::ProcessAsciiControlChar(char cc)
{
    // We do not support All C0 codes
    Superclass::ProcessAsciiControlChar(cc);
}
