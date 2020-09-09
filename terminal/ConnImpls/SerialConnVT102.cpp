//
// (c) 2020 chiv
//
#include "terminal_rc.h"
#include "SerialConnVT102.h"
#include "VT102ControlSeq.h"
#include "ConnFactory.h"
// register
REGISTER_CONN_INSTANCE("VT102", SerialConnVT102);
//
using namespace Upp;

SerialConnVT102::SerialConnVT102(std::shared_ptr<SerialIo> serial)
    : Superclass(serial)
    , mSS(VT102_SI)
    , mCharset(VT102_US)
{
    // Save cursor data, this is the default cursor data
    SaveCursor(mCursorData);
    // Install VT102 functions
    InstallVT102Functions();
}

SerialConnVT102::~SerialConnVT102()
{
}

int SerialConnVT102::IsControlSeq(const std::string& seq)
{
    int ret = IsVT102ControlSeq(seq);
    if (ret == 0) {
        ret = Superclass::IsControlSeq(seq);
    }
    return ret;
}
//----------------------------------------------------------------------------------------------
void SerialConnVT102::SaveCursor(CursorData& cd)
{
    cd.Vx = mVx;
    cd.Vy = mVy;
    cd.FgColor = mFgColor;
    cd.BgColor = mBgColor;
    cd.Font = mFont;
    cd.Blink = mBlink;
}

void SerialConnVT102::LoadCursor(const CursorData& cd)
{
    mVx = cd.Vx;
    mVy = cd.Vy;
    mFont = cd.Font;
    mFgColor = cd.FgColor;
    mBgColor = cd.BgColor;
    mBlink = cd.Blink;
    //
    DoLayout();
}
//----------------------------------------------------------------------------------------------
std::string SerialConnVT102::TranscodeToUTF8(const VTChar& cc) const
{
    return Utf32ToUtf8(VT102_Transcode(cc, mCharset, mSS));
}
//----------------------------------------------------------------------------------------------
void SerialConnVT102::InstallVT102Functions()
{
    // 1. ANSI Compatible Seq
    // 1.1 set mode
    mVT102TrivialHandlers["[2h"] = [=]() {
        mModes.KeyboardAction = VT102Modes::Locked;
    };
    mVT102TrivialHandlers["[4h"] = [=]() {
        mModes.InsertionReplacement = VT102Modes::Insert;
    };
    mVT102TrivialHandlers["[12h"] = [=]() {
        mModes.SendReceive = VT102Modes::Off;
    };
    mVT102TrivialHandlers["[20h"] = [=]() {
        mModes.LineFeedNewLine = VT102Modes::NewLine;
    };
    mVT102TrivialHandlers["[?1h"] = [=]() {
        mModes.CursorKey = VT102Modes::Application;
    };
    mVT102TrivialHandlers["[?3h"] = [=]() {
        mModes.Column = VT102Modes::C132;
    };
    mVT102TrivialHandlers["[?4h"] = [=]() {
        mModes.Scrolling = VT102Modes::Smooth;
    };
    mVT102TrivialHandlers["[?5h"] = [=]() {
        if (mModes.Screen == VT102Modes::Normal) {
            mPaperColor = Color(~mPaperColor.GetR(), ~mPaperColor.GetG(), ~mPaperColor.GetB());
            mModes.Screen = VT102Modes::Reverse;
        }
    };
    mVT102TrivialHandlers["[?6h"] = [=]() {
        mModes.Origin = VT102Modes::Relative;
    };
    mVT102TrivialHandlers["[?7h"] = [=]() {
        mModes.AutoWrap = VT102Modes::On; // Ignore this, we have unlimited line length.
    };
    mVT102TrivialHandlers["[?8h"] = [=]() {
        mModes.AutoRepeat = VT102Modes::On;
        // clear key stats
        mKeyStats.clear();
    };
    mVT102TrivialHandlers["[?18h"] = [=]() {
        mModes.PrintFormFeed = VT102Modes::On;
    };
    mVT102TrivialHandlers["[?19h"] = [=]() {
        mModes.PrintExtent = VT102Modes::FullScreen;
    };
    // 1.2 reset mode
    mVT102TrivialHandlers["[2l"] = [=]() {
        mModes.KeyboardAction = VT102Modes::Unlocked;
    };
    mVT102TrivialHandlers["[4l"] = [=]() {
        mModes.InsertionReplacement = VT102Modes::Replace;
    };
    mVT102TrivialHandlers["[12l"] = [=]() {
        mModes.SendReceive = VT102Modes::On;
    };
    mVT102TrivialHandlers["[20l"] = [=]() {
        mModes.LineFeedNewLine = VT102Modes::LineFeed;
    };
    mVT102TrivialHandlers["[?1l"] = [=]() {
        mModes.CursorKey = VT102Modes::Cursor;
    };
    mVT102TrivialHandlers["[?3l"] = [=]() {
        mModes.Column = VT102Modes::C80;
    };
    mVT102TrivialHandlers["[?4l"] = [=]() {
        mModes.Scrolling = VT102Modes::Jump;
    };
    mVT102TrivialHandlers["[?5l"] = [=]() {
        if (mModes.Screen == VT102Modes::Reverse) {
            mPaperColor = Color(~mPaperColor.GetR(), ~mPaperColor.GetG(), ~mPaperColor.GetB());
            mModes.Screen = VT102Modes::Normal;
        }
    };
    mVT102TrivialHandlers["[?6l"] = [=]() {
        mModes.Origin = VT102Modes::Absolute;
    };
    mVT102TrivialHandlers["[?7l"] = [=]() {
        mModes.AutoWrap = VT102Modes::Off;
    };
    mVT102TrivialHandlers["[?8l"] = [=]() {
        mModes.AutoRepeat = VT102Modes::Off;
    };
    mVT102TrivialHandlers["[?18l"] = [=]() {
        mModes.PrintFormFeed = VT102Modes::Off;
    };
    mVT102TrivialHandlers["[?19l"] = [=]() {
        mModes.PrintExtent = VT102Modes::ScrollingRegion;
    };
    // 1.3 Cursor Key Codes Generated [Application]
    mVT102TrivialHandlers["[0A"] = [=]() {
        ProcessVT102CursorMovementCmds("[A");
    };
    mVT102TrivialHandlers["[0B"] = [=]() {
        ProcessVT102CursorMovementCmds("[B");
    };
    mVT102TrivialHandlers["[0C"] = [=]() {
        ProcessVT102CursorMovementCmds("[C");
    };
    mVT102TrivialHandlers["[0D"] = [=]() {
        ProcessVT102CursorMovementCmds("[D");
    };
    // 1.4 Keypad character selection
    // 1.5 Keypad codes generated
    // 1.6 select character sets
    mVT102TrivialHandlers["(A"] = [=]() {
        mSS = VT102_SI; // shift in, G0
        mCharset = VT102_UK;
    };
    mVT102TrivialHandlers[")A"] = [=]() {
        mSS = VT102_SO; // shift out, G1
        mCharset = VT102_UK;
    };
    mVT102TrivialHandlers["(B"] = [=]() {
        mSS = VT102_SI; // shift in, G0
        mCharset = VT102_US;
    };
    mVT102TrivialHandlers[")B"] = [=]() {
        mSS = VT102_SO; // shift out, G1
        mCharset = VT102_US;
    };
    mVT102TrivialHandlers["(0"] = [=]() {
        mSS = VT102_SI; // shift in, G0
        mCharset = VT102_SpecialChars_LineDrawing;
    };
    mVT102TrivialHandlers[")0"] = [=]() {
        mSS = VT102_SO; // shift out, G1
        mCharset = VT102_SpecialChars_LineDrawing;
    };
    mVT102TrivialHandlers["(1"] = [=]() {
        mSS = VT102_SI; // shift in, G0
        mCharset = VT102_ROM;
    };
    mVT102TrivialHandlers[")1"] = [=]() {
        mSS = VT102_SO; // shift out, G1
        mCharset = VT102_ROM;
    };
    mVT102TrivialHandlers["(2"] = [=]() {
        mSS = VT102_SI; // shift in, G0
        mCharset = VT102_ROM_SpecialChars;
    };
    mVT102TrivialHandlers[")2"] = [=]() {
        mSS = VT102_SO; // shift out, G1
        mCharset = VT102_ROM_SpecialChars;
    };
    // 1.7 character attributes, see IsVT102Attrs
    // 1.8 scrolling region, see IsVT102ScrollingRegion
    // 1.9 Cursor movement commands, those seq with parameters were processed by
    // ProcessVT102CursorMovementCmds.
    mVT102TrivialHandlers["[H"] = [=]() { // cursor position (home)
        // home
        mScrollToEnd = true;
        if (mModes.Origin == VT102Modes::Relative) {
            mVy = mScrollingRegion.Top;
        } else {
            mVy = 0;
        }
        mVx = 0;
    };
    mVT102TrivialHandlers["[f"] = mVT102TrivialHandlers["[H"];
    mVT102TrivialHandlers["E"] = [=]() { // next line
        mVy++;
        mVx = 0;
    };
    mVT102TrivialHandlers["D"] = [=]() { // index
        mVy++;
    };
    mVT102TrivialHandlers["M"] = [=]() { // reverse index
        mVy--;
        if (mVy < 0) mVy = 0;
    };
    mVT102TrivialHandlers["7"] = [=]() { SaveCursor(mCursorData); };
    mVT102TrivialHandlers["8"] = [=]() { LoadCursor(mCursorData); };
    // 1.10 Tab stops
    mVT102TrivialHandlers["H"] = [=]() { // HTS
        this->ProcessAsciiControlChar(0x09);
    };
    mVT102TrivialHandlers["[g"] = [=]() { // TBC , clear tab
        if (mLines[mVy][mVx] == '\t') { // remove it.
            mLines[mVy].erase(mLines[mVy].begin() + mVx);
            mLines[mVy].push_back(mBlankChar);
        }
    };
    mVT102TrivialHandlers["[0g"] = mVT102TrivialHandlers["[g"];
    mVT102TrivialHandlers["[3g"] = [=]() { // clear all tabs
        for (size_t i = 0; i < mLines.size(); ++i) {
            VTLine vline;
            for (size_t j = 0; j < mLines[i].size(); ++j) {
                if (mLines[i][j] != '\t') {
                    vline.push_back(mLines[i][j]);
                }
            }
            for (size_t k = vline.size(); k < mLines[i].size(); ++k) {
                vline.push_back(' ');
            }
            mLines[i] = std::move(vline);
        }
    };
    // 1.11 Line attributes, we do not implement line attributes
    // 1.12 Erasing
    mVT102TrivialHandlers["[K"] = [=]() { // erase in line, cursor to end of line
        if (mVy < (int)mLines.size()) {
            Size csz = GetConsoleSize();
            for (int i = mVx; i < csz.cx; ++i) {
                mLines[mVy][i] = mBlankChar;
            }
        }
    };
    mVT102TrivialHandlers["[0K"] = mVT102TrivialHandlers["[K"];
    mVT102TrivialHandlers["[1K"] = [=]() { // erase in line, beginning of line to cursor
        if (mVy < (int)mLines.size()) {
            Size csz = GetConsoleSize();
            for (int i = 0; i < mVx; ++i) {
                mLines[mVy][i] = mBlankChar;
            }
        }
    };
    mVT102TrivialHandlers["[2K"] = [=]() { // erase in line, entire line containing cursor
        Size csz = GetConsoleSize();
        if (mVy < (int)mLines.size()) {
            mLines[mVy] = VTLine(csz.cx, mBlankChar).SetHeight(mFontH);
        }
    };
    mVT102TrivialHandlers["[J"] = [=]() { // erase in display, cursor to end of screen
        Size csz = GetConsoleSize();
        if (mVy < (int)mLines.size()) {
            // csz.cy == mLines.size(), this condition is always true.
            for (int i = mVx; i < csz.cx; ++i) {
                mLines[mVy][i] = mBlankChar;
            }
            for (int i = mVy+1; i < csz.cy; ++i) {
                mLines[i] = VTLine(csz.cx, mBlankChar).SetHeight(mFontH);
            }
        }
    };
    mVT102TrivialHandlers["[0J"] = mVT102TrivialHandlers["[J"];
    mVT102TrivialHandlers["[1J"] = [=]() { // erase in display, beginning of screen to cursor
        if (mVy < (int)mLines.size()) {
            Size csz = GetConsoleSize();
            for (int i = 0; i < mVx; ++i) {
                mLines[mVy][i] = mBlankChar;
            }
            for (int i = 0; i < mVy; ++i) {
                mLines[i] = VTLine(csz.cx, mBlankChar).SetHeight(mFontH);
            }
        }
    };
    mVT102TrivialHandlers["[2J"] = [=]() { // erase in display, entire screen
        Size csz = GetConsoleSize();
        for (size_t k = 0; k < mLines.size(); ++k) {
            mLines[k] = VTLine(csz.cx, mBlankChar).SetHeight(mFontH);
        }
    };
    // 1.13 Editing functions, see IsVT102EditingFunctions
    // 1.14 Print commands, we do not support printer.
    // 1.15 Reports
    mVT102TrivialHandlers["[5n"] = [=]() {
        GetSerial()->Write("\033[0n");
    };
    mVT102TrivialHandlers["[?15n"] = [=]() { // status of printer
        // report that: No printer
        GetSerial()->Write("\033[?13n");
    };
    mVT102TrivialHandlers["[6n"] = [=]() {
        std::string rsp = "\033["
                          + std::to_string(mVy+1) + ";"
                          + std::to_string(mVx+1) + "R";
        GetSerial()->Write(rsp);
    };
    mVT102TrivialHandlers["[c"] = [=]() {
        GetSerial()->Write("\033[?6c"); // VT102
    };
    mVT102TrivialHandlers["[0c"] = mVT102TrivialHandlers["[c"]; // ESC Z is not recommended, we do not support it.
    // 1.16 Reset
    mVT102TrivialHandlers["c"] = [=]() { Clear(); };
    // 1.17 Test and adjustments
    mVT102TrivialHandlers["#8"] = [=]() {
        Size csz = GetConsoleSize();
        for (size_t i = 0; i < mLines.size(); ++i) {
            mLines[i] = VTLine(csz.cx, 'E').SetHeight(mFontH);
        }
    };
    // 1.18 keyboard led
    // 2. VT52 Compatible Mode
    // 2.1 keypad character selection
    // 2.2 character sets
    // 2.3 cursor position
    // others, see IsVT52CursorKeyCodes
    //-----------------------------------------------------
    // 2.4 Erasing
    // 2.5 Print Commands
}
//
void SerialConnVT102::ProcessVT102CursorMovementCmds(const std::string& seq)
{
    if (seq.length() < 2) return; // do nothing
    switch (*seq.rbegin()) {
    case 'A':if (1) {
        int n = atoi(seq.substr(1, seq.length() - 2).c_str());
        if (n <= 0) n = 1;
        mVy -= n;
        mVy = std::max(mVy, 0);
    } break;
    case 'B':if (1) {
        int n = atoi(seq.substr(1, seq.length() - 2).c_str());
        if (n <= 0) n = 1;
        mVy += n;
    } break;
    case 'C':if (1) {
        int n = atoi(seq.substr(1, seq.length() - 2).c_str());
        if (n <= 0) n = 1;
        mVx += n;
    } break;
    case 'D':if (1) {
        int n = atoi(seq.substr(1, seq.length() - 2).c_str());
        if (n <= 0) n = 1;
        mVx -= n;
        mVx = std::max(mVx, 0);
    } break;
    case 'H':
    case 'f':if (1) {
        auto p = seq.find(';');
            if (p != seq.npos) {
            int y = atoi(seq.substr(1, p-1).c_str()) - 1;
            int x = atoi(seq.substr(p+1, seq.size() - p - 1).c_str()) - 1;
            Size csz = GetConsoleSize();
            if (x < 0) x = 0; mVx = x; if (mVx >= csz.cx) mVx = csz.cx-1;
            if (y < 0) y = 0; mVy = y; if (mVy >= csz.cy) mVy = csz.cy-1;
        }
    } break;
    }
}
//
void SerialConnVT102::ProcessVT102ScrollingRegion(const std::string& seq)
{
    if (seq.length() < 3) return; else {
        size_t p = seq.find(';');
        if (p == seq.npos) return;
        // ok, let's go, t,b count from 1, so we sub 1 here.
        int t = atoi(seq.substr(1, p-1).c_str()) - 1;                     // top
        int b = atoi(seq.substr(p+1, seq.size() - p - 1).c_str()) - 1;    // bottom
        if (t >= 0 && b - t > 1 && t < (int)mLines.size()) {
            mScrollingRegion.Top = t;
            mScrollingRegion.Bottom = b;
            // Home to scrolling region.
            mVx = 0; mVy = t;
        }
    }
}
//
void SerialConnVT102::ProcessVT102EditingFunctions(const std::string& seq)
{
    if (seq.length() < 2) return;
    switch (*seq.rbegin()) {
    case 'P':if (1) { // delete character
        if (mVy >= (int)mLines.size()) break;
        const int n_chars = atoi(seq.substr(1, seq.length()-2).c_str());
        int n_moved = 0;
        const int& y = mVy;
        int p = mVx + n_chars;
        if (p < (int)mLines[y].size()) {
            for (int i = mVx; n_moved < n_chars && i < (int)mLines[y].size(); ++i) {
                mLines[y][i] = mLines[y][p+i];
                n_moved++;
            }
        } else {
            for (int i = mVx; n_moved < n_chars && i < (int)mLines[y].size(); ++i) {
                mLines[y][i] = mBlankChar;
            }
        }
    } break;
    case 'L':if (1) { // insert line
        
    } break;
    case 'M':if (1) { // delete line
    } break;
    }
}
//
void SerialConnVT102::ProcessVT102CharAttribute(int attr_code)
{
    switch (attr_code) {
    case 0: // Default
        mCurrAttrFuncs.clear();
        mCurrAttrFuncs.push_back([=](){ SetDefaultStyle(); });
        break;
    case 1: // bold
        mCurrAttrFuncs.push_back([=]() { mFont.Bold(); });
        break;
    case 4: // underline
        mCurrAttrFuncs.push_back([=]() { mFont.Underline(); });
        break;
    case 5: // blink
        mCurrAttrFuncs.push_back([=]() { mBlink = true; });
        break;
    case 7: // reverse
        mCurrAttrFuncs.push_back([=]() { std::swap(mFgColor, mBgColor); });
        break;
    }
}
//
void SerialConnVT102::ProcessVT102CharAttributes(const std::string& seq)
{
    std::string s = seq.substr(1, seq.size()-1);
    if (s.empty()) {
        ProcessVT102CharAttribute(0);
    } else {
        SplitString(std::move(s), ';', [=](const char* s) { ProcessVT102CharAttribute(atoi(s)); });
    }
}
//
void SerialConnVT102::ProcessVT102Trivial(const std::string& seq)
{
    auto it = mVT102TrivialHandlers.find(seq);
    if (it != mVT102TrivialHandlers.end()) {
        it->second();
    }
}
//
void SerialConnVT102::ProcessControlSeq(const std::string& seq, int seq_type)
{
    if (seq_type == VT102_Trivial) {
        ProcessVT102Trivial(seq);
    } else if (seq_type > VT102_Trivial && seq_type < VT102_SeqType_Endup) {
        switch (seq_type) {
        case VT52_CursorKeyCodes: // DOES NOT SUPPORT VT52
            break;
        case VT102_CharAttributes: ProcessVT102CharAttributes(seq); break;
        case VT102_CursorMovementCmds: ProcessVT102CursorMovementCmds(seq); break;
        case VT102_EditingFunctions: ProcessVT102EditingFunctions(seq); break;
        case VT102_ScrollingRegion: ProcessVT102ScrollingRegion(seq); break;
        // ▒ stands for a canceled control seq.
        case VT102_CanceledSeq: if (1) {
            size_t ep;RenderText(Utf8ToUtf32("▒", ep));
        } break;
        default:break;
        }
    } else {
        Superclass::ProcessControlSeq(seq, seq_type);
    }
}
//
bool SerialConnVT102::ProcessAsciiControlChar(char cc)
{
    switch (cc) {
    case 0x00: break; // NUL
    case 0x03: break; // ETX
    case 0x04: break; // EOT
    case 0x05: break; // ENQ
    case 0x07: break; // BEL
    case 0x0a: // LF, Also causes printing if auto print operation selected.
               // We do not support printer, just process the linefeed/newline.
    case 0x0b: // VT, vertical tab
    case 0x0c: // FF
        if (mModes.LineFeedNewLine == VT102Modes::NewLine) {
            // LF,FF,VT - cursor moves to left margin of next line
            mVy += 1;
            mVx = 0;
        } else {
            // LF,FF,VT - cursor moves to next line but stays in same column
            mVy += 1;
        }
        break;
    // WE DO NOT NEED THESE, because we support them in other ways.
    case 0x0e: mSS = VT102_SO; break; // SO
    case 0x0f: mSS = VT102_SI; break; // SI
    case 0x11: break; // DC1
    case 0x13: break; // DC3
    case 0x18: // CAN
    case 0x1a: // SUB, processed as CAN
        break;
    default: return Superclass::ProcessAsciiControlChar(cc);
    }
    return true;
}
//----------------------------------------------
bool SerialConnVT102::ProcessKeyDown(dword key, dword flags)
{
    if (ShouldIgnoreKey(key)) return true;
    std::string d;
    // https://vt100.net/docs/vt102-ug/chapter4.html#T4-1
    if ((flags & (K_CTRL | K_ALT | K_SHIFT)) == 0) {
        switch (key) {
        case K_LEFT:
            d = mModes.CursorKey == VT102Modes::Cursor ? "\x1b[D" : std::string("\x1b") + "0D";
            break;
        case K_RIGHT:
            d = mModes.CursorKey == VT102Modes::Cursor ? "\x1b[C" : std::string("\x1b") + "0C";
            break;
        case K_UP:
            d = mModes.CursorKey == VT102Modes::Cursor ? "\x1b[A" : std::string("\x1b") + "0A";
            break;
        case K_DOWN:
            d = mModes.CursorKey == VT102Modes::Cursor ? "\x1b[B" : std::string("\x1b") + "0B";
            break;
        case K_ESCAPE:
            d = "\x1b";
            break;
        case K_HOME:
            d = "\x1b[H"; // Home in line.
            break;
        default:break;
        }
    } else if ((flags & (K_CTRL | K_SHIFT)) == (K_CTRL | K_SHIFT)) {
        switch (key) {
        case K_GRAVE: // CTRL+~
            d = "\x1e";
            break;
        case K_SLASH: // CTRL+?
            d = "\x1f";
            break;
        }
    } else if (flags & K_CTRL) {
        switch (key) {
        case K_SPACE:
            d = "\x00";
            break;
        }
    }
    if (!d.empty()) {
        GetSerial()->Write(d);
        return true;
    } else return Superclass::ProcessKeyDown(key, flags);
}

bool SerialConnVT102::ProcessKeyUp(dword key, dword flags)
{
    auto k_ = mKeyStats.find(key);
    if (k_ != mKeyStats.end()) {
        mKeyStats.clear();
        return true;
    }
    return Superclass::ProcessKeyUp(key, flags);
}

bool SerialConnVT102::ShouldIgnoreKey(Upp::dword key)
{
    if (mModes.AutoRepeat == VT102Modes::Off) {
        auto k_ = mKeyStats.find((int)key);
        if (k_ != mKeyStats.end() && k_->second) {
            return true; // Please ignore this key.
        } else {
            mKeyStats[key] = true; // Pressed.
        }
    }
    return false;
}

bool SerialConnVT102::ProcessChar(Upp::dword cc)
{
    if (ShouldIgnoreKey(cc)) return true;
    if (mModes.LineFeedNewLine == VT102Modes::NewLine) {
        // If in NewLine mode, send CRLF when RETURN was pressed.
        if (cc == 0x0d) {
            GetSerial()->Write("\r\n");
            return true;
        }
    }
    if (isprint((int)cc) && mModes.SendReceive == VT102Modes::On) {
        // local echo
        RenderText(std::vector<uint32_t>({(uint32_t)cc}));
    }
    return Superclass::ProcessChar(cc);
}
