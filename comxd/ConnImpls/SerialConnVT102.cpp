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
    : SerialConn(serial)
    , mCursorX(0)
    , mCursorY(0)
    , mRxShouldStop(false)
    , mFgColor(Color(255, 255, 255))
    , mBgColor(Color(110, 110, 0))
    , mPaperColor(Color(110, 110, 0))
    , mDefaultBgColor(Color(110, 110, 0))
    , mDefaultFgColor(Color(255, 255, 255))
    , mBlinkSignal(true)
    , mBlink(false)
    , mVisible(true)
    , mScrollToEnd(true)
    , mPressed(false)
    , mMaxLinesBufferSize(5000)
{
    mActOptions = UsrAction(comxd::about(), "Options", "Change console options", [=](){
        ShowOptionsDialog();
    });
    // double buffer
    BackPaint();
    // default font
    mFont = Upp::Monospace();
    // handlers
    mFontW = mFont.GetWidth('M');
    mFontH = mFont.GetLineHeight();
    // default style
    mCurrAttrFuncs.push_back([=]() { SetDefaultStyle(); });
    // enable scroll bar
    mSbV.WhenScroll = [=]() {
        int pp = mSbV.Get() / mFontH;
        int pg = mSbV.GetPage() / mFontH;
        int pt = mSbV.GetTotal() / mFontH;
        mScrollToEnd = pp + pg == pt;
        Refresh();
    };
    mSbV.SetLine(mFontH); mSbV.Set(0); mSbV.SetTotal(0);
    AddFrame(mSbV);
    // Install handlers
    InstallVT102ControlSeqHandlers();
    //
    SetTimeCallback(-500, [&]() {
        mBlinkSignal = !mBlinkSignal;
        this->Refresh();
        }, kBlinkTimerId);
    //
    mBlankChr = ' ';
    mBlankChr.SetAttrFuns(mCurrAttrFuncs);
    //
    mRxThr = std::thread([=]() { RxProc(); });
}

SerialConnVT102::~SerialConnVT102()
{
    mRxShouldStop = true;
    if (mRxThr.joinable()) {
        mRxThr.join();
    }
    KillTimeCallback(kBlinkTimerId);
}
//
void SerialConnVT102::SetDefaultStyle()
{
    mFont.NoBold().NoItalic().NoUnderline();
    mBgColor = mDefaultBgColor;
    mFgColor = mDefaultFgColor;
    mBlink = false;
}
// receiver
void SerialConnVT102::RxProc()
{
    bool pending = false; // If pending is true, that stands for a VT102 seq is pending
                          // we should treat the successive characters as a part of last
                          // VT102 seq.
    std::string pattern;
    while (!mRxShouldStop) {
        size_t sz = mSerial->available();
        if (sz) {
            std::string pretty_buff;
            std::vector<uint8_t> buff;
            GetSerial()->read(buff, sz);
            for (size_t k = 0; k < buff.size(); ++k) {
                if (pending) {
#if 0 // Support VT102 CANCEL(0x18)
                    if (buff[k] == 0x18) {
                        pretty_buff.push_back('@'); // replace it with @
                        pending = false;
                        pattern = "";
                        continue;
                    }
#endif
                    pattern.push_back((char)buff[k]);
                    int ret = IsControlSeq(pattern);
                    if (ret >= 2) { // bingo
                        // found it.
                        Upp::PostCallback([=]() { RenderText("\x1b" + pattern, ret); });
                        //
                        pending = false;
                        pattern = "";
                    } else if (ret == 0) { // no, it's not
                        Upp::PostCallback([=]() { RenderText("\\x1b" + pattern, -1); });
                        pending = false;
                        pattern = "";
                    }
                } else if (buff[k] == 0x1b) {
                    // come across a 0x1b, render the pretty_buff
                    if (!pretty_buff.empty()) {
                        Upp::PostCallback([=]() { RenderText(pretty_buff, -1); });
                    }
                    pretty_buff = "";
                    pending = true; // continue
                } else {
                    pretty_buff.push_back((char)buff[k]);
                }
            }
            // render the pretty_buff
            if (!pretty_buff.empty()) {
                Upp::PostCallback([=]() { RenderText(pretty_buff, -1); });
            }
            // callback is not safe
            Upp::PostCallback([=]() { this->Refresh(); });
        } else std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
    }
}

std::list<const UsrAction*> SerialConnVT102::GetActions() const
{
    std::list<const UsrAction*> actions({&mActOptions});
    return actions;
}

int SerialConnVT102::IsControlSeq(const std::string& seq)
{
    return IsVT102ControlSeq(seq);
}

void SerialConnVT102::ShowOptionsDialog()
{
    // TODO: show options dialog
    auto sz = GetConsoleSize();
    std::string text = std::to_string(sz.cx) + ":" + std::to_string(sz.cy);
    PromptOK(text.c_str());
}

Size SerialConnVT102::GetConsoleSize() const
{
    Size sz = GetSize();
    // monospace
    ASSERT(mFont.GetFaceInfo() & Font::FIXEDPITCH);
    
    return Upp::Size(sz.cx / mFontW, sz.cy / mFontH);
}
//
void SerialConnVT102::PushToLinesBufferAndCheck(const VTLine& vline)
{
    mLinesBuffer.push_back(vline);
    if (mLinesBuffer.size() > mMaxLinesBufferSize) {
        mLinesBuffer.erase(mLinesBuffer.begin());
    }
}
//
void SerialConnVT102::InstallVT102ControlSeqHandlers()
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
    mCtrlHandlers["[H"] = [=]() { // cursor position (home)
        // home
        mScrollToEnd = true;
        mCursorX = 0;
        mCursorY = 0;
    };
    mCtrlHandlers["[f"] = mCtrlHandlers["[H"];
    // 1.10 Tab stops
    // 1.11 Line attributes
    // 1.12 Erasing
    mCtrlHandlers["[K"] = [=]() { // erase in line, cursor to end of line
        Size csz = GetConsoleSize();
        for (int i = mCursorX; i < csz.cx; ++i) {
            mLines[mCursorY][i] = mBlankChr;
        }
    };
    mCtrlHandlers["[0K"] = mCtrlHandlers["[K"];
    mCtrlHandlers["[1K"] = [=]() { // erase in line, beginning of line to cursor
        Size csz = GetConsoleSize();
        for (int i = 0; i < mCursorX; ++i) {
            mLines[mCursorY][i] = mBlankChr;
        }
    };
    mCtrlHandlers["[2K"] = [=]() { // erase in line, entire line containing cursor
        Size csz = GetConsoleSize();
        mLines[mCursorY] = VTLine(csz.cx, mBlankChr);
    };
    mCtrlHandlers["[J"] = [=]() { // erase in display, cursor to end of screen
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
    mCtrlHandlers["[0J"] = mCtrlHandlers["[J"];
    mCtrlHandlers["[1J"] = [=]() { // erase in display, beginning of screen to cursor
        Size csz = GetConsoleSize();
        for (int i = 0; i < mCursorX; ++i) {
            mLines[mCursorY][i] = mBlankChr;
        }
        for (int i = 0; i < mCursorY; ++i) {
            mLines[i] = VTLine(csz.cx, mBlankChr);
        }
    };
    mCtrlHandlers["[2J"] = [=]() { // erase in display, entire screen
        Size csz = GetConsoleSize();
        for (size_t k = 0; k < mLines.size(); ++k) {
            mLines[k] = VTLine(csz.cx, mBlankChr);
        }
    };
    // 1.13 Editing functions, see IsVT102EditingFunctions
    // 1.14 Print commands
    // 1.15 Reports
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
        mCursorY += n;
        mCursorY = std::min(mCursorY, csz.cy - 1);
    } break;
    case 'B':if (1) {
        int n = atoi(seq.substr(1, seq.length() - 2).c_str());
        if (n <= 0) n = 1;
        mCursorY -= n;
        mCursorY = std::max(mCursorY, 0);
    } break;
    case 'C':if (1) {
        int n = atoi(seq.substr(1, seq.length() - 2).c_str());
        if (n <= 0) n = 1;
        mCursorX += n;
        mCursorX = std::min(mCursorX, csz.cx-1);
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
void SerialConnVT102::ProcessAttr(const std::string& attr_code)
{
    int m = atoi(attr_code.c_str());
    switch (m) {
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
void SerialConnVT102::ProcessVT102Attrs(const std::string& seq)
{
    if (seq == "[m") ProcessAttr("0"); else {
        // [ Pn;Pn;Pn;Pn;Pn...m
        size_t p = 1; // skip '['
        auto q = seq.find(';', p);
        while (q != seq.npos) {
            std::string m = seq.substr(p, q-p);
            ProcessAttr(m);
            p = q+1;
            // next q
            if (q >= seq.size()-1) break; else {
                q = seq.find(';', p);
            }
        }
        if (p < seq.size()-1) {
            ProcessAttr(seq.substr(p, seq.size()-p-1));
        }
    }
}
//
void SerialConnVT102::ProcessControlSeq(const std::string& seq, int seq_type)
{
    if (seq_type == 2) {
        auto it = mCtrlHandlers.find(seq);
        if (it != mCtrlHandlers.end()) {
            it->second();
        }
    } else {
        switch (seq_type) {
        case VT52_Cursor: // DOES NOT SUPPORT VT52
            break;
        case VT102_Attrs: ProcessVT102Attrs(seq); break;
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
void SerialConnVT102::ProcessOverflowLines()
{
    Size csz = GetConsoleSize();
    if (mCursorY >= csz.cy) { // scroll
        size_t cn = mCursorY - csz.cy + 1;
        for (size_t k = 0; k < cn; ++k) {
            PushToLinesBufferAndCheck(*mLines.begin());
            mLines.erase(mLines.begin());
            mLines.push_back(VTLine(csz.cx, mBlankChr));
        }
        mCursorY = csz.cy - 1; // move to last line
    }
}
void SerialConnVT102::RenderText(const std::string& seq, int seq_type)
{
    std::lock_guard<std::mutex> _(mLinesLock);
    Size csz = GetConsoleSize();
    if (seq[0] != 0x1b) { // Not VT102 control seq
        for (size_t k = 0; k < seq.size(); ++k) {
            VTChar chr = seq[k];
            if (isprint(chr)) {
                // chr always uses the last attr funcs
                chr.SetAttrFuns(mCurrAttrFuncs);
                //
                VTLine& vline = mLines[mCursorY];
                if (mCursorX < vline.size()) {
                    vline[mCursorX++] = chr;
                } else {
                    // wrap lines longer than screen width
                    mCursorY++;
                    mCursorX = 0;
                    ProcessOverflowLines();
                    vline[mCursorX++] = chr;
                }
            } else {
                ProcessAsciiControlChar(chr);
            }
            ProcessOverflowLines();
        }
    } else {
        ProcessControlSeq(seq.substr(1), seq_type);
    }
}

void SerialConnVT102::Paint(Upp::Draw& draw)
{
    // draw background
    draw.DrawRect(this->GetRect(), Color(0, 0, 0));
    //
    Render(draw);
}

Upp::Image SerialConnVT102::CursorImage(Point p, dword keyflags)
{
    (void)p;
    (void)keyflags;
    //
    return Image::IBeam();
}

void SerialConnVT102::MouseWheel(Point, int zdelta, dword)
{
	mSbV.Wheel(zdelta);
}

void SerialConnVT102::MouseMove(Upp::Point p, Upp::dword)
{
    if (mPressed) {
        int nlines = (int)mLinesBuffer.size() + \
            (int)mLines.size() - this->CalculateNumberOfBlankLinesFromEnd(mLines);
        mSelectionSpan.X1 = p.x / mFontW;
        int y = (mSbV.Get() + p.y) / mFontH; // absolute position
        if (y >= nlines) {
            mSelectionSpan.Y1 = nlines - 1;
        } else {
            mSelectionSpan.Y1 = y;
        }
        if (p.y <= 0) {
            mSbV.PrevLine();
        } else if (p.y > GetSize().cy) {
            mSbV.NextLine();
        }
        // fix it
        if (mSelectionSpan.X0 < 0) mSelectionSpan.X0 = 0;
        if (mSelectionSpan.X1 < 0) mSelectionSpan.X1 = 0;
        if (mSelectionSpan.Y0 < 0) mSelectionSpan.Y0 = 0;
        if (mSelectionSpan.Y1 < 0) mSelectionSpan.Y1 = 0;
        //
        Refresh();
    }
}

void SerialConnVT102::MouseLeave()
{
}

void SerialConnVT102::RightUp(Point, dword)
{
	MenuBar::Execute(
		[=](Bar& bar) {
		    String text1 = GetSelectedText();
			bar.Add(text1.GetCount() > 0, t_("Copy"), [=] {
			    // copy to clipboard
			    Upp::AppendClipboardText(text1);
			});
			String text2 = Upp::ReadClipboardText();
			bar.Add(text2.GetCount() > 0, t_("Paste"), [=] {
			    GetSerial()->write(text2.ToStd());
			});
		}
	);
}

void SerialConnVT102::LeftUp(Point p, dword)
{
    (void)p;
    if (mPressed) {
        mPressed = false;
        Refresh();
    }
}

void SerialConnVT102::LeftDown(Point p, dword)
{
    int nlines = (int)mLinesBuffer.size() + \
        (int)mLines.size() - this->CalculateNumberOfBlankLinesFromEnd(mLines);
    mPressed = true;
    mSelectionSpan.X0 = p.x / mFontW;
    mSelectionSpan.Y0 = (mSbV.Get() + p.y) / mFontH;
    mSelectionSpan.X1 = p.x / mFontW;
    mSelectionSpan.Y1 = (mSbV.Get() + p.y) / mFontH;
    int maxy = std::max(mSelectionSpan.Y0, mSelectionSpan.Y1);
    if (maxy >= nlines) {
        if (mSelectionSpan.Y1 > mSelectionSpan.Y0) {
            mSelectionSpan.Y1 = nlines-1;
            mSelectionSpan.Y0 = mSelectionSpan.Y1;
        } else {
            mSelectionSpan.Y0 = nlines-1;
            mSelectionSpan.Y1 = mSelectionSpan.Y0;
        }
    }
    SetFocus();
    SetCapture();
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

bool SerialConnVT102::Key(dword key, int)
{
    bool processed = false;
	// keydown
	if (key & K_DELTA) {
	    dword flags = K_CTRL | K_ALT | K_SHIFT;
	    dword d_key = key & ~(flags | K_KEYUP); // key with delta
	    flags = key & flags;
	    if (key & K_KEYUP) {
	        processed = ProcessKeyUp(d_key, flags);
	    } else {
	        processed = ProcessKeyDown(d_key, flags);
	    }
	} else {
	    dword flags = K_CTRL | K_ALT | K_SHIFT | K_KEYUP;
	    flags = key & flags;
	    dword d_key = key & ~flags;
	    if (d_key > 0 && d_key < 0x7f) {
	        processed = ProcessKeyDown_Ascii(key, flags);
	    }
	}
	if (processed) {
	    mScrollToEnd = true;
	}
	return processed;
}
//
int SerialConnVT102::CalculateNumberOfBlankLinesFromEnd(const std::vector<SerialConnVT102::VTLine>& lines) const
{
    int cn = 0;
    size_t sz = lines.size();
    while (sz--) {
        const VTLine& vline = lines[sz];
        bool blank = true;
        for (size_t k = 0; k < vline.size(); ++k) {
            if (vline[k] != ' ') {
                blank = false;
                break;
            }
        }
        if (blank) {
            cn++;
        } else break;
    }
    return cn;
}

int SerialConnVT102::CalculateNumberOfBlankCharsFromEnd(const VTLine& vline) const
{
    int cn = 0;
    size_t sz = vline.size();
    while (sz--) {
        if (vline[sz] == ' ')
            cn++;
        else break;
    }
    return cn;
}

void SerialConnVT102::Layout()
{
	Size csz = GetConsoleSize();
	mSbV.SetPage(mFontH*csz.cy);
	// check and fix
	for (size_t k = 0; k < mLines.size(); ++k) {
	    VTLine& vline = mLines[k];
	    for (size_t i = vline.size(); (int)i < csz.cx; ++i) {
	        vline.push_back(mBlankChr);
	    }
	}
	// extend or shrink the virtual screen
	if (mLines.size() < csz.cy) {
	    // extend virtual screen
	    // 0. pull lines from lines buffer
	    int extcn = csz.cy - (int)mLines.size();
	    int ln = 0;
	    for (int i = 0; i < extcn && !mLinesBuffer.empty(); ++i) {
	        VTLine vline = *mLinesBuffer.rbegin(); mLinesBuffer.pop_back();
	        for (int n = (int)vline.size(); n < csz.cx; ++n) {
	            vline.push_back(mBlankChr);
	        }
	        mLines.insert(mLines.begin(), vline);
	        ln++;
	    }
	    mCursorY += ln;
	    // 1. push blanks
	    for (int i = ln; i < csz.cy && i < extcn; ++i) {
	        mLines.push_back(VTLine(csz.cx, mBlankChr));
	    }
	    //--------------------------------------------------------------------------------------
	} else {
	    // shrink virtual screen
	    // 0. remove blanks firstly
	    int blankcnt = CalculateNumberOfBlankLinesFromEnd(mLines);
	    int shkcn = (int)mLines.size() - csz.cy;
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
	mScrollToEnd = true;
}

std::vector<SerialConnVT102::VTLine> SerialConnVT102::GetBufferLines(size_t p, int& y)
{
    Size csz = GetConsoleSize(); y = mCursorY;
    std::vector<VTLine> out(csz.cy);
    // fetch lines from lines buffer
    size_t ln = 0;
    for (size_t k = p; k < mLinesBuffer.size() && (int)ln < csz.cy; ++k) {
        out[ln] = mLinesBuffer[k];
        for (int i = out[ln].size(); i < csz.cx; ++i) {
            out[ln].push_back(mBlankChr);
        }
        ln++;
        y++;
    }
    int cn = 0;
    for (size_t k = ln; (int)k < csz.cy; ++k) {
        out[ln++] = mLines[cn++];
    }
    if (y >= (int)mLines.size()) {
        y = -1;
    }
    
    return out;
}
//
std::vector<std::string> SerialConnVT102::GetSelection() const
{
    auto span = mSelectionSpan;
    if (span.Y0 > span.Y1) { // top left
        std::swap(span.Y0, span.Y1);
        std::swap(span.X0, span.X1);
    } else if (span.Y0 == span.Y1) {
        if (span.X0 > span.X1) {
            std::swap(span.X0, span.X1);
        }
    }
    int i, j;
    std::vector<std::string> out;
    auto templines = mLinesBuffer;
    int nlines = (int)mLines.size() - this->CalculateNumberOfBlankLinesFromEnd(mLines);
    for (int i = 0; i < nlines; ++i) {
        templines.push_back(mLines[i]);
    }
    //
    if (span.Y0 >= templines.size()) return out;
    // first line
    if (1) {
        std::string line;
        int first_ep = span.Y0 == span.Y1 ? span.X1 : (int)templines[span.Y0].size()-1;
        for (i = span.X0; i <= first_ep; ++i) {
            line.push_back(templines[span.Y0][i]);
        }
        out.push_back(line);
    }
    // entire lines
    for (i = span.Y0+1; i < span.Y1 && i < (int)templines.size(); ++i) {
        auto& vline = templines[i];
        std::string line;
        for (int j = 0; j < (int)vline.size(); ++j) {
            line.push_back(vline[j]);
        }
        out.push_back(line);
    }
    // tail line
    if (i == span.Y1 && i < (int)templines.size()) {
        auto& vline = templines[i];
        std::string line;
        for (int j = 0; j <= span.X1; ++j) {
            line.push_back(vline[j]);
        }
        out.push_back(line);
    }
    return out;
}
//
String SerialConnVT102::GetSelectedText() const
{
    String out;
    auto lines = GetSelection();
    for (size_t k = 0; k < lines.size(); ++k) {
        if (k != 0) {
            out += "\n";
        }
        // strip tail blanks of lines
        size_t n = 0;
        while (lines[k][lines[k].size()-1-n] == ' ' && n < lines[k].size()) {
            n++;
        }
        out += lines[k].substr(0, lines[k].size() - n);
    }
    return out;
}
//
bool SerialConnVT102::IsCharInSelectionSpan(int x, int y) const
{
    auto span = mSelectionSpan;
    if (span.Y0 > span.Y1) { // top left
        std::swap(span.Y0, span.Y1);
        std::swap(span.X0, span.X1);
    } else if (span.Y0 == span.Y1) {
        if (span.X0 > span.X1) {
            std::swap(span.X0, span.X1);
        }
    }
    // calculate absolute position
    if (span.X1 - span.X0 <= 0 &&
        span.Y1 - span.Y0 <= 0) return false;
    int abs_x = x, abs_y = mSbV.Get() / mFontH + y;
    if (abs_y == span.Y0) { // head line
        if (span.Y1-span.Y0 == 0) {
            if (abs_x >= span.X0 && abs_x <= span.X1) return true;
        } else {
            return abs_x >= span.X0;
        }
    } else if (abs_y == span.Y1) {
        if (abs_x <= span.X1) return true;
    } else {
        if (abs_y > span.Y0 && abs_y < span.Y1) {
            return true;
        }
    }
    return false;
}

void SerialConnVT102::Render(Upp::Draw& draw)
{
    // draw background
    draw.DrawRect(GetRect(), mPaperColor);
    // set total
    int nlines = (int)mLinesBuffer.size() + \
        (int)mLines.size();// - this->CalculateNumberOfBlankLinesFromEnd(mLines);
    mSbV.SetTotal(mFontH * nlines);
    // draw VT102 chars
    if (mScrollToEnd) { // draw current screen
        draw.DrawRect(mCursorX*mFontW, mCursorY*mFontH, mFontW, mFontH, Color(0, 255, 0));
        mSbV.End(); // scroll to end
        // draw text
        Render(draw, mLines);
    } else { // draw buffered screen
        int y = -1;
        int p = mSbV / mFontH;
        auto vlines = GetBufferLines(p, y);
        if (y > 0) {
            draw.DrawRect(mCursorX*mFontW, y*mFontH, mFontW, mFontH, Color(0, 255, 0));
        }
        Render(draw, vlines);
    }
}

void SerialConnVT102::Render(Upp::Draw& draw, const std::vector<VTLine>& vlines)
{
    Size csz = GetConsoleSize();
    // from 0
    mX = 0;
    mY = 0;
    int nlines = (int)mLines.size() - this->CalculateNumberOfBlankLinesFromEnd(vlines);
    for (size_t k = 0; k < nlines; ++k) {
        const VTLine& vline = vlines[k];
        for (size_t i = 0; i < vline.size() && i < csz.cx; ++i) {
            vline[i].ApplyAttrs();
            bool is_selected = IsCharInSelectionSpan((int)i, (int)k);
            if (is_selected) {
                std::swap(mBgColor, mFgColor);
            }
            const char buff[2] = {vline[i], '\0'};
            if (mBlink) {
                if (mBlinkSignal) {
                    draw.DrawRect(mX, mY, mFontW, mFontH, mPaperColor);
                } else {
                    if (mBgColor != mDefaultBgColor) {
                        draw.DrawRect(mX, mY, mFontW, mFontH, mBgColor);
                    }
                    if (mFgColor != mBgColor) {
                        if (buff[0] != ' ' && mVisible) {
                            draw.DrawText(mX, mY, buff, mFont, mFgColor);
                        }
                    }
                }
            } else {
                if (mBgColor != mDefaultBgColor) {
                    draw.DrawRect(mX, mY, mFontW, mFontH, mBgColor);
                }
                if (mFgColor != mBgColor) {
                    if (buff[0] != ' ' && mVisible) {
                        draw.DrawText(mX, mY, buff, mFont, mFgColor);
                    }
                }
            }
            mX += mFontW;
            if (is_selected) {
                std::swap(mBgColor, mFgColor);
            }
        }
        // apply attrs for those lines longer than screen width
        for (size_t i = csz.cx; i < vline.size(); ++i) {
            vline[i].ApplyAttrs();
        }
        // move to next line
        mX = 0;
        mY += mFontH;
    }
}

