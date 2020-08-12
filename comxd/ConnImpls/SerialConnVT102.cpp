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
}

SerialConnVT102::~SerialConnVT102()
{
}

int SerialConnVT102::IsControlSeq(const std::string& seq)
{
    return IsVT102ControlSeq(seq);
}
//
void SerialConnVT102::InstallVT102Functions()
{
    // 1. ANSI Compatible Seq
    // 1.1 set mode
    // 1.2 reset mode
    // 1.3 Cursor Key Codes Generated, see IsVT102CursorKeyCodes
    // 1.4 Keypad character selection
    // 1.5 Keypad codes generated
    // 1.6 select character sets
    // 1.7 character attributes, see IsVT102Attrs
    // 1.8 scrolling region, see IsVT102ScrollingRegion
    // 1.9 Cursor movement commands
    mVT102TrivialHandlers["[H"] = [=]() { // cursor position (home)
        // home
        mScrollToEnd = true;
        mCursorX = 0;
        mCursorY = 0;
    };
    mVT102TrivialHandlers["[f"] = mVT102TrivialHandlers["[H"];
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
    // "\033Z" is not recommended
    mVT102TrivialHandlers["[?6c"] = [=]() {
        GetSerial()->write("VT102");
    };
    // 1.16 Reset
    // 1.17 Test and adjustments
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
void SerialConnVT102::ProcessVT102CursorKeyCodes(const std::string& seq)
{
    Size csz = GetConsoleSize();
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
        ASSERT(p != seq.npos);
        int x = atoi(seq.substr(1, p-1).c_str());
        int y = atoi(seq.substr(p+1, seq.size() - p - 1).c_str());
        if (x < 0) x = 0; if (x >= csz.cx) x = csz.cx - 1;
        if (y < 0) y = 0; if (y >= csz.cy) y = csz.cy - 1;
        mCursorX = x;
        mCursorY = y;
    } break;
    }
}
//
void SerialConnVT102::ProcessVT102EditingFunctions(const std::string& seq)
{
    switch (*seq.rbegin()) {
    case 'P':if (1) { // delete character
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
        case VT52_Cursor: // DOES NOT SUPPORT VT52
            break;
        case VT102_Attrs: ProcessVT102CharAttributes(seq); break;
        case VT102_Cursor: ProcessVT102CursorKeyCodes(seq); break;
        case VT102_EditingFunctions: ProcessVT102EditingFunctions(seq); break;
        case VT102_ScrollingRegion: // DOES NOT SUPPORT THIS FEATURE
            break;
        }
    }
}
//
void SerialConnVT102::ProcessAsciiControlChar(unsigned char cc)
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
    std::vector<uint8_t> d;
    if ((flags & (K_CTRL | K_ALT | K_SHIFT)) == 0) {
        switch (key) {
        case K_BACKSPACE:
            d.push_back(8);
            break;
        case K_LEFT:
            d.push_back(0x1b);
            d.push_back('[');
            d.push_back('D');
            break;
        case K_RIGHT:
            d.push_back(0x1b);
            d.push_back('[');
            d.push_back('C');
            break;
        case K_UP:
            d.push_back(0x1b);
            d.push_back('[');
            d.push_back('A');
            break;
        case K_DOWN:
            d.push_back(0x1b);
            d.push_back('[');
            d.push_back('B');
            break;
        case K_ESCAPE:
            d.push_back(0x1b);
            break;
        case K_DELETE:
            //
            break;
        case K_HOME:
            d = std::vector<uint8_t>({0x1b, '[', 'H'});
            break;
        case K_END:
            // ?
            break;
        default:break;
        }
    } else if ((flags & K_CTRL) == K_CTRL) {
        // https://vt100.net/docs/vt102-ug/chapter4.html#T4-1
        key = key & ~K_CTRL;
        if (key >= K_A && key <= K_Z) {
            d.push_back(1 + key - K_A);
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
