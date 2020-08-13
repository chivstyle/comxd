//
// (c) 2020 chiv
//
#include "resource.h"
#include "SerialConnVT102.h"
#include "VT102ControlSeq.h"
#include "ConnFactory.h"
// register
REGISTER_CONN_INSTANCE("VT102", SerialConnVT102);
//
using namespace Upp;

SerialConnVT102::SerialConnVT102(std::shared_ptr<serial::Serial> serial)
    : Superclass(serial)
{
    InstallVT102Functions();
    // save cursor firstly.
    SaveCursor(mCursorData);
}

SerialConnVT102::~SerialConnVT102()
{
}

int SerialConnVT102::IsControlSeq(const std::string& seq)
{
    return IsVT102ControlSeq(seq);
}
//----------------------------------------------------------------------------------------------
void SerialConnVT102::SaveCursor(CursorData& cd)
{
    cd.X = mCursorX;
    cd.Y = mCursorY;
    cd.FgColor = mFgColor;
    cd.BgColor = mBgColor;
    cd.Bold = mFont.IsBold();
    cd.Italic = mFont.IsItalic();
    cd.Strikeout = mFont.IsStrikeout();
    cd.Underline = mFont.IsUnderline();
}

void SerialConnVT102::LoadCursor(const CursorData& cd)
{
    mCursorX = cd.X;
    mCursorY = cd.Y;
    mFont.Bold(cd.Bold);
    mFont.Italic(cd.Italic);
    mFont.Strikeout(cd.Strikeout);
    mFont.Underline(cd.Underline);
    mFgColor = cd.FgColor;
    mBgColor = cd.BgColor;
    //
    DoLayout();
}
//----------------------------------------------------------------------------------------------
// These overrides, support ScrollingRegion
Size SerialConnVT102::GetConsoleSize() const
{
    return Superclass::GetConsoleSize();
}
// If the scrolling region was set, we should ignore those lines out of region
void SerialConnVT102::ProcessOverflowLines()
{
    auto& span = mScrollingRegion;
    Size csz = GetConsoleSize();
    int bottom = span.Bottom;
    if (bottom < 0 || bottom >= csz.cy)
        bottom = csz.cy-1;
    ASSERT(span.Top < (int)mLines.size());
    if (mCursorY == bottom+1) {
        PushToLinesBufferAndCheck(mLines[span.Top]);
        mLines.erase(mLines.begin() + span.Top);
        mLines.insert(mLines.begin() + bottom, VTLine(csz.cx, mBlankChr));
        mCursorY = bottom;
    } else if (mCursorY >= csz.cy) {
        mCursorY = csz.cy - 1;
    }
}
//
void SerialConnVT102::DrawCursor(Upp::Draw& draw, int vx, int vy)
{
    Size csz = GetConsoleSize();
    int top = mScrollingRegion.Top;
    int bot = mScrollingRegion.Bottom;
    if (bot < 0) bot = csz.cy - 1;
    if (vy >= 0 && vy <= bot) {
        int x = -mSbH.Get() + mFontW*vx;
        int y = mFontH*vy;
        draw.DrawRect(x, y, mFontW, mFontH, Color(0, 255, 0));
    }
}
//
void SerialConnVT102::ExtendVirtualScreen(int cx, int cy)
{
    // extend virtual screen
    int bot = mScrollingRegion.Bottom;
    if (bot < 0) {
        bot = cy;
    }
    // 0. pull lines from lines buffer
    int ln = 0;
    int extcn = cy - (int)mLines.size();
    if (bot >= cy) {
        for (int i = 0; i < extcn && !mLinesBuffer.empty(); ++i) {
            VTLine vline = *mLinesBuffer.rbegin(); mLinesBuffer.pop_back();
            for (int n = (int)vline.size(); n < cx; ++n) {
                vline.push_back(mBlankChr);
            }
            mLines.insert(mLines.begin(), vline);
            ln++;
        }
        mCursorY += ln;
    }
    // 1. push blanks
    for (int i = ln; i < cy && i < extcn; ++i) {
        mLines.push_back(VTLine(cx, mBlankChr));
    }
}

void SerialConnVT102::ShrinkVirtualScreen(int cx, int cy)
{
    // shrink virtual screen
    // Shrink will destroy the scrolling region
    mScrollingRegion.Top = 0;
    mScrollingRegion.Bottom = -1;
    // 0. remove blanks firstly
    int blankcnt = CalculateNumberOfBlankLinesFromEnd(mLines);
    int shkcn = (int)mLines.size() - cy;
    int ln = 0;
    for (int i = 0; i < shkcn && i < blankcnt; ++i) {
        mLines.pop_back();
        ln++;
    }
    // 1. remove from head
    for (int i = ln; i < shkcn; ++i) {
        PushToLinesBufferAndCheck(*mLines.begin());
        mLines.erase(mLines.begin());
        if (mCursorY > 0) {
            mCursorY--;
        }
    }
}
//
void SerialConnVT102::DoLayout()
{
    Size csz = GetConsoleSize();
	mSbV.SetPage(mFontH*csz.cy);
	mSbH.SetPage(mFontW*csz.cx);
	if (csz.cx <= 0 || csz.cy <= 0) return;
	// check and fix
	for (size_t k = 0; k < mLines.size(); ++k) {
	    VTLine& vline = mLines[k];
	    for (size_t i = vline.size(); (int)i < csz.cx; ++i) {
	        vline.push_back(mBlankChr);
	    }
	}
	// extend or shrink the virtual screen
	if (mLines.size() < csz.cy) {
	    ExtendVirtualScreen(csz.cx, csz.cy);
	    //--------------------------------------------------------------------------------------
	} else {
	    ShrinkVirtualScreen(csz.cx, csz.cy);
	}
}
//
std::vector<SerialConnVT::VTLine> SerialConnVT102::GetMergedScreen(size_t p, int& nlines_from_buffer) const
{
    Size csz = GetConsoleSize();
    std::vector<VTLine> out(csz.cy);
    size_t ln = 0;
    // -----------------------------------------------------------------------------------------
    //  part 0,    0 ~ Top
    //  part 1,    p ~ buffer lines, max count is (bot-top), denote as sn
    //  part 3,    top ~ bot - sn, include bot-sn
    //  part 4,    bot ~ end of lines
    //  for example, top = 9, bot = 19, it defines a scrolling range, 11 lines total
    //------------------------------------------------------------------------------------------
    int top = mScrollingRegion.Top;
    int bot = mScrollingRegion.Bottom;
    if (bot < 0) bot = csz.cy - 1;
    // part 0
    for (int k = 0; k < (int)mLines.size() && k < top; ++k) {
        out[ln++] = mLines[k];
    }
    // part 1
    int sn = 0;
    for (size_t k = p; k < mLinesBuffer.size() && ln < out.size() && sn <= bot-top; ++k) {
        VTLine vline = mLinesBuffer[k];
        for (int i = (int)vline.size(); i < csz.cx; ++i) {
            vline.push_back(mBlankChr);
        }
        out[ln++] = vline;
        sn++;
    }
    nlines_from_buffer = sn;
    // part 3
    for (int k = top; k <= bot - sn && ln < out.size(); ++k) {
        out[ln++] = mLines[k];
    }
    for (size_t k = bot + 1; k < out.size() && ln < out.size(); ++k) {
        out[ln++] = mLines[k];
    }
    
    return out;
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
        mModes.Screen = VT102Modes::Reverse;
    };
    mVT102TrivialHandlers["[?6h"] = [=]() {
        mModes.Origin = VT102Modes::Relative;
    };
    mVT102TrivialHandlers["[?7h"] = [=]() {
        mModes.AutoWrap = VT102Modes::On;
    };
    mVT102TrivialHandlers["[?8h"] = [=]() {
        mModes.AutoRepeat = VT102Modes::On;
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
        mModes.Screen = VT102Modes::Normal;
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
    // 1.7 character attributes, see IsVT102Attrs
    // 1.8 scrolling region, see IsVT102ScrollingRegion
    // 1.9 Cursor movement commands, those seq with parameters were processed by
    // ProcessVT102CursorMovementCmds.
    mVT102TrivialHandlers["[H"] = [=]() { // cursor position (home)
        // home
        mScrollToEnd = true;
        mCursorX = 0;
        mCursorY = 0;
    };
    mVT102TrivialHandlers["[f"] = mVT102TrivialHandlers["[H"];
    mVT102TrivialHandlers["E"] = [=]() { // next line
        mCursorY++;
        mCursorX = 0;
    };
    mVT102TrivialHandlers["D"] = [=]() { // index
        mCursorY++;
    };
    mVT102TrivialHandlers["M"] = [=]() { // reverse index
        mCursorY--;
        if (mCursorY < 0) mCursorY = 0;
    };
    mVT102TrivialHandlers["7"] = [=]() { SaveCursor(mCursorData); };
    mVT102TrivialHandlers["8"] = [=]() { LoadCursor(mCursorData); };
    // 1.10 Tab stops
    // 1.11 Line attributes
    // 1.12 Erasing
    mVT102TrivialHandlers["[K"] = [=]() { // erase in line, cursor to end of line
        Size csz = GetConsoleSize();
        for (int i = mCursorX; i < csz.cx; ++i) {
            mLines[mCursorY][i] = mBlankChr;
        }
    };
    mVT102TrivialHandlers["[0K"] = mVT102TrivialHandlers["[K"];
    mVT102TrivialHandlers["[1K"] = [=]() { // erase in line, beginning of line to cursor
        Size csz = GetConsoleSize();
        for (int i = 0; i < mCursorX; ++i) {
            mLines[mCursorY][i] = mBlankChr;
        }
    };
    mVT102TrivialHandlers["[2K"] = [=]() { // erase in line, entire line containing cursor
        Size csz = GetConsoleSize();
        mLines[mCursorY] = VTLine(csz.cx, mBlankChr);
    };
    mVT102TrivialHandlers["[J"] = [=]() { // erase in display, cursor to end of screen
        // clear entire screen, push lines to buffer.
        if (mCursorX == 0 && mCursorY == 0) {
            int nlines = (int)mLines.size() - this->CalculateNumberOfBlankLinesFromEnd(mLines);
            for (int i = 0; i < nlines; ++i) {
                this->PushToLinesBufferAndCheck(mLines[i]);
            }
        }
        Size csz = GetConsoleSize();
        for (int i = mCursorX; i < csz.cx; ++i) {
            mLines[mCursorY][i] = mBlankChr;
        }
        for (int i = mCursorY+1; i < csz.cy; ++i) {
            mLines[i] = VTLine(csz.cx, mBlankChr);
        }
    };
    mVT102TrivialHandlers["[0J"] = mVT102TrivialHandlers["[J"];
    mVT102TrivialHandlers["[1J"] = [=]() { // erase in display, beginning of screen to cursor
        Size csz = GetConsoleSize();
        for (int i = 0; i < mCursorX; ++i) {
            mLines[mCursorY][i] = mBlankChr;
        }
        for (int i = 0; i < mCursorY; ++i) {
            mLines[i] = VTLine(csz.cx, mBlankChr);
        }
    };
    mVT102TrivialHandlers["[2J"] = [=]() { // erase in display, entire screen
        Size csz = GetConsoleSize();
        for (size_t k = 0; k < mLines.size(); ++k) {
            mLines[k] = VTLine(csz.cx, mBlankChr);
        }
    };
    // 1.13 Editing functions, see IsVT102EditingFunctions
    // 1.14 Print commands
    // 1.15 Reports
    mVT102TrivialHandlers["[5n"] = [=]() {
        GetSerial()->write("\033[0n");
    };
    mVT102TrivialHandlers["[?15n"] = [=]() { // status of printer
        // report that: No printer
        GetSerial()->write("\033[?13n");
    };
    mVT102TrivialHandlers["[6n"] = [=]() {
        std::string rsp = "\033["
                          + std::to_string(mCursorY+1) + ";"
                          + std::to_string(mCursorX+1) + "R";
        GetSerial()->write(rsp);
    };
    mVT102TrivialHandlers["[c"] = [=]() {
        GetSerial()->write("\033[?6c"); // VT102
    };
    mVT102TrivialHandlers["[0c"] = mVT102TrivialHandlers["[c"]; // ESC Z is not recommended, we do not support it.
    // 1.16 Reset
    mVT102TrivialHandlers["c"] = [=]() { Clear(); };
    // 1.17 Test and adjustments
    mVT102TrivialHandlers["#8"] = [=]() {
        Size csz = GetConsoleSize();
        for (size_t k = 0; k < mLines.size(); ++k) {
            mLines[k] = VTLine(mBlankChr, csz.cx);
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
        mCursorY -= n;
        mCursorY = std::max(mCursorY, 0);
    } break;
    case 'B':if (1) {
        int n = atoi(seq.substr(1, seq.length() - 2).c_str());
        if (n <= 0) n = 1;
        mCursorY += n;
    } break;
    case 'C':if (1) {
        int n = atoi(seq.substr(1, seq.length() - 2).c_str());
        if (n <= 0) n = 1;
        mCursorX += n;
    } break;
    case 'D':if (1) {
        int n = atoi(seq.substr(1, seq.length() - 2).c_str());
        if (n <= 0) n = 1;
        mCursorX -= n;
        mCursorX = std::max(mCursorX, 0);
    } break;
    case 'H':
    case 'f':if (1) {
        auto p = seq.find(';');
            if (p != seq.npos) {
            int y = atoi(seq.substr(1, p-1).c_str()) - 1;
            int x = atoi(seq.substr(p+1, seq.size() - p - 1).c_str()) - 1;
            Size csz = GetConsoleSize();
            if (x < 0) x = 0; mCursorX = x; if (mCursorX >= csz.cx) mCursorX = csz.cx-1;
            if (y < 0) y = 0; mCursorY = y; if (mCursorY >= csz.cy) mCursorY = csz.cy-1;
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
            // Home
            mCursorX = 0; mCursorY = t;
        }
    }
}
//
void SerialConnVT102::ProcessVT102EditingFunctions(const std::string& seq)
{
    if (seq.length() < 2) return;
    switch (*seq.rbegin()) {
    case 'P':if (1) { // delete character
        if (mCursorY >= (int)mLines.size()) break;
        const int n_chars = atoi(seq.substr(1, seq.length()-2).c_str());
        int n_moved = 0;
        const int& y = mCursorY;
        int p = mCursorX + n_chars;
        if (p < (int)mLines[y].size()) {
            for (int i = mCursorX; n_moved < n_chars && i < (int)mLines[y].size(); ++i) {
                mLines[y][i] = mLines[y][p+i];
                n_moved++;
            }
        } else {
            for (int i = mCursorX; n_moved < n_chars && i < (int)mLines[y].size(); ++i) {
                mLines[y][i] = mBlankChr;
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
        }
    }
}
//
void SerialConnVT102::ProcessAsciiControlChar(char cc)
{
    switch (cc) {
    case '\r':
        mCursorX = 0;
        break;
    case '\n':
        mCursorY += 1;
        break;
    case '\t':
        mCursorX += 4;
        break;
    case 0x08: // backspace
        if (mCursorX > 0) {
            mCursorX -= 1;
        }
        break;
    case 0x07: { // bell
    } break;
    }
}
//----------------------------------------------
bool SerialConnVT102::ProcessKeyDown(dword key, dword flags)
{
    std::string d;
    if ((flags & (K_CTRL | K_ALT | K_SHIFT)) == 0) {
        switch (key) {
        case K_BACKSPACE:
            d.push_back(8);
            break;
        case K_LEFT:
            d = "\033[D";
            break;
        case K_RIGHT:
            d = "\033[C";
            break;
        case K_UP:
            d = "\033[A";
            break;
        case K_DOWN:
            d = "\033[B";
            break;
        case K_ESCAPE:
            d = "\033";
            break;
        case K_HOME:
            d = "\033[H"; // Home in line.
            break;
        default:break;
        }
    } else if ((flags & K_CTRL) == K_CTRL) {
        // https://vt100.net/docs/vt102-ug/chapter4.html#T4-1
        key = key & ~K_CTRL;
        if (key >= K_A && key <= K_Z) {
            d.push_back((char)(1 + key - K_A));
        }
    }
    if (!d.empty()) {
        GetSerial()->write(d);
        return true;
    }
    return false;
}

bool SerialConnVT102::ProcessKeyUp(dword key, dword flags)
{
    return false;
}

bool SerialConnVT102::ProcessKeyDown_Ascii(Upp::dword key, Upp::dword flags)
{
    std::vector<uint8_t> d;
    if (flags == 0) {
        d.push_back((uint8_t)(key & 0xff));
    } else if ((flags & K_CTRL) == K_CTRL) {
        // https://vt100.net/docs/vt102-ug/chapter4.html#T4-1
        switch (key) {
        case '[':
            d.push_back(0x1b); // ESC
            break;
        case '/':
            d.push_back(0x1c); // FS
            break;
        case ']':
            d.push_back(0x1d); // GS
            break;
        case '~':
            d.push_back(0x1e); // RS
            break;
        case '?':
            d.push_back(0x1f); // US
            break;
        case ' ':
            d.push_back('\0'); // NUL
            break;
        }
    }
    if (!d.empty()) {
        GetSerial()->write(d);
        return true;
    }
    return false;
}
