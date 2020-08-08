//
// (c) 2020 chiv
//
#include "resource.h"
#include "SerialConnVT.h"
#include "ConnFactory.h"
#include <algorithm>
// register
using namespace Upp;
//
SerialConnVT::SerialConnVT(std::shared_ptr<serial::Serial> serial)
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
    mSbV.SetLine(mFontH); mSbV.Set(0); mSbV.SetTotal(0); AddFrame(mSbV);
    mSbH.WhenScroll = [=]() {
        Refresh();
    };
    mSbH.SetLine(mFontW); mSbH.Set(0); mSbV.SetTotal(0); AddFrame(mSbH);
    //
#if 0
    SetTimeCallback(-500, [&]() {
        mBlinkSignal = !mBlinkSignal;
        this->Refresh();
        }, kBlinkTimerId);
#endif
    mBlankChr = ' ';
    mBlankChr.SetAttrFuns(mCurrAttrFuncs);
    //
    mRxThr = std::thread([=]() { RxProc(); });
}

SerialConnVT::~SerialConnVT()
{
    mRxShouldStop = true;
    if (mRxThr.joinable()) {
        mRxThr.join();
    }
    KillTimeCallback(kBlinkTimerId);
}
//
void SerialConnVT::SetDefaultStyle()
{
    mFont.NoBold().NoItalic().NoUnderline();
    mBgColor = mDefaultBgColor;
    mFgColor = mDefaultFgColor;
    mBlink = false;
}
// receiver
void SerialConnVT::RxProc()
{
    bool pending = false; // If pending is true, that stands for a VT seq is pending
                          // we should treat the successive characters as a part of last
                          // VT seq.
    std::string pattern, raw;
    while (!mRxShouldStop) {
        size_t sz = mSerial->available();
        if (sz) {
            std::vector<unsigned char> buff; GetSerial()->read(buff, sz);
            for (size_t k = 0; k < buff.size(); ++k) {
                if (pending) {
                    pattern.push_back((char)buff[k]);
                    int ret = IsControlSeq(pattern);
                    if (ret >= 2) { // bingo
                        // found it.
                        Upp::PostCallback([=]() { ProcessControlSeq(pattern, ret); });
                        pending = false;
                        pattern = "";
                    } else if (ret == 0) { // can not recognize it
                        size_t ep; auto s = Utf8ToUtf32("\\x1b" + pattern, ep);
                        Upp::PostCallback([=]() { RenderText(s); });
                        pending = false;
                        pattern = "";
                    }
                } else if (buff[k] == 0x1b) {
                    // come across a 0x1b, render the raw
                    if (!raw.empty()) {
                        size_t ep; auto s = Utf8ToUtf32(raw, ep);
                        raw = raw.substr(ep);
                        Upp::PostCallback([=]() { RenderText(s); });
                    }
                    pending = true; // continue
                } else {
                    raw.push_back(buff[k]);
                }
            }
            if (!raw.empty()) {
                size_t ep; auto s = Utf8ToUtf32(raw, ep);
                raw = raw.substr(ep);
                Upp::PostCallback([=]() { RenderText(s); });
            }
            // callback is
            Upp::PostCallback([=]() { this->Refresh(); });
        } else std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
    }
}

std::list<const UsrAction*> SerialConnVT::GetActions() const
{
    std::list<const UsrAction*> actions({&mActOptions});
    return actions;
}

void SerialConnVT::ShowOptionsDialog()
{
    // TODO: show options dialog
    auto sz = GetConsoleSize();
    std::string text = std::to_string(sz.cx) + ":" + std::to_string(sz.cy);
    PromptOK(text.c_str());
}

int SerialConnVT::GetCharWidth(const VTChar& c) const
{
    int cx = mFont.GetWidth((int)c);
    // adjust width
    return (cx + mFontW-1) / mFontW * mFontW;
}

Size SerialConnVT::GetConsoleSize() const
{
    Size sz = GetSize();
    // monospace
    ASSERT(mFont.GetFaceInfo() & Font::FIXEDPITCH);
    if (sz.cx <= 0 || sz.cy <= 0) {
        int cx = mLines.empty() ? 0 : (int)mLines[0].size();
        int cy = (int)mLines.size();
        return Upp::Size(cx, cy);
    }
    return Upp::Size(sz.cx / mFontW, sz.cy / mFontH);
}
//
int SerialConnVT::LogicToVirtual(const VTLine& vline, int lx) const
{
    int x = 0;
    for (size_t k = 0; k < vline.size(); ++k) {
        x += GetCharWidth(vline[k]);
        if (x == lx) return (int)k+1; else if (x > lx) {
            return (int)k;
        }
    }
    return 0;
}
//
int SerialConnVT::VirtualToLogic(const VTLine& vline, int vx) const
{
    int x = 0;
    for (size_t k = 0; k < vx && k < vline.size(); ++k) {
        x += GetCharWidth(vline[k]);
    }
    return x;
}
//
int SerialConnVT::GetLogicWidth(const VTLine& vline) const
{
    int x = 0;
    for (size_t k = 0; k < vline.size(); ++k) {
        x += GetCharWidth(vline[k]);
    }
    return x;
}
//
void SerialConnVT::PushToLinesBufferAndCheck(const VTLine& vline)
{
    mLinesBuffer.push_back(vline);
    if (mLinesBuffer.size() > mMaxLinesBufferSize) {
        mLinesBuffer.erase(mLinesBuffer.begin());
    }
}
//
void SerialConnVT::ProcessAsciiControlChar(unsigned char cc)
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
void SerialConnVT::ProcessOverflowLines()
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
void SerialConnVT::RenderText(const std::vector<uint32_t>& s)
{
    Size csz = GetConsoleSize();
    for (size_t k = 0; k < s.size(); ++k) {
        VTChar chr = s[k];
        if (chr < 0x20 || chr == 0x7f) {
            ProcessAsciiControlChar(chr);
        } else {
            chr.SetAttrFuns(mCurrAttrFuncs);
            //
            VTLine& vline = mLines[mCursorY];
            if (mCursorX < vline.size()) {
                vline[mCursorX++] = chr;
            } else {
                // extend the vline
                vline.push_back(chr);
            }
        }
        ProcessOverflowLines();
    }
}

void SerialConnVT::Paint(Upp::Draw& draw)
{
    // draw background
    draw.DrawRect(this->GetRect(), Color(0, 0, 0));
    //
    Render(draw);
}

Upp::Image SerialConnVT::CursorImage(Point p, dword keyflags)
{
    (void)p;
    (void)keyflags;
    //
    return Image::IBeam();
}

void SerialConnVT::MouseWheel(Point, int zdelta, dword)
{
	mSbV.Wheel(zdelta);
}

void SerialConnVT::MouseMove(Upp::Point p, Upp::dword)
{
    if (mPressed) {
        int nlines = (int)mLinesBuffer.size() + \
            (int)mLines.size() - this->CalculateNumberOfBlankLinesFromEnd(mLines);
        mSelectionSpan.X1 = (mSbH.Get() + p.x) / mFontW;
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
        if (p.x <= 0) {
            mSbH.PrevLine();
        } else if (p.x > GetSize().cx) {
            mSbH.NextLine();
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

void SerialConnVT::MouseLeave()
{
}

void SerialConnVT::RightUp(Point, dword)
{
	MenuBar::Execute(
		[=](Bar& bar) {
		    String text1 = GetSelectedText();
			bar.Add(text1.GetCount() > 0, t_("Copy"), [=] {
			    // copy to clipboard
			    Upp::AppendClipboardText(text1);
			});
			String text2 = Upp::ReadClipboardUnicodeText().ToString();
			bar.Add(text2.GetCount() > 0, t_("Paste"), [=] {
			    GetSerial()->write(text2.ToStd());
			});
		}
	);
}

void SerialConnVT::LeftUp(Point p, dword)
{
    (void)p;
    if (mPressed) {
        mPressed = false;
        Refresh();
    }
}

void SerialConnVT::LeftDown(Point p, dword)
{
    int nlines = (int)mLinesBuffer.size() + \
        (int)mLines.size() - this->CalculateNumberOfBlankLinesFromEnd(mLines);
    mPressed = true;
    mSelectionSpan.X0 = (mSbH.Get() + p.x) / mFontW;
    mSelectionSpan.Y0 = (mSbV.Get() + p.y) / mFontH;
    mSelectionSpan.X1 = (mSbH.Get() + p.x) / mFontW;
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
bool SerialConnVT::ProcessKeyDown(dword key, dword flags)
{
    std::vector<uint8_t> d;
    if ((flags & (K_CTRL | K_ALT | K_SHIFT)) == 0) {
        switch (key) {
        case K_BACKSPACE:
            d.push_back(8);
            break;
        case K_ESCAPE:
            d.push_back(0x1b);
            break;
        default:break;
        }
    } else if ((flags & K_CTRL) == K_CTRL) {
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
bool SerialConnVT::ProcessKeyUp(dword key, dword flags)
{
    return false;
}
bool SerialConnVT::ProcessKeyDown_Ascii(Upp::dword key, Upp::dword flags)
{
    std::vector<uint8_t> d;
    if (flags == 0) {
        d.push_back((uint8_t)(key & 0xff));
    } else if ((flags & K_CTRL) == K_CTRL) {
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

bool SerialConnVT::Key(dword key, int)
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
int SerialConnVT::CalculateNumberOfBlankLinesFromEnd(const std::vector<SerialConnVT::VTLine>& lines) const
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

int SerialConnVT::CalculateNumberOfBlankCharsFromEnd(const VTLine& vline) const
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

void SerialConnVT::Layout()
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

std::vector<SerialConnVT::VTLine> SerialConnVT::GetBufferLines(size_t p, int& y)
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
std::vector<std::string> SerialConnVT::GetSelection() const
{
    //------------------------------------------------------------------------------------------
    auto span = mSelectionSpan;
    if (span.Y0 > span.Y1) { // top left
        std::swap(span.Y0, span.Y1);
        std::swap(span.X0, span.X1);
    } else if (span.Y0 == span.Y1) {
        if (span.X0 > span.X1) {
            std::swap(span.X0, span.X1);
        }
    }
    std::vector<std::string> out;
    auto templines = mLinesBuffer;
    int nlines = (int)mLines.size() - this->CalculateNumberOfBlankLinesFromEnd(mLines);
    for (int i = 0; i < nlines; ++i) {
        templines.push_back(mLines[i]);
    }
    //------------------------------------------------------------------------------------------
    for (int k = span.Y0; k <= span.Y1 && k < (int)templines.size(); ++k) {
        const VTLine& vline = templines[k];
        std::string sel;
        for (size_t i = 0; i < vline.size(); ++i) {
            bool is_selected = IsCharInSelectionSpan(VirtualToLogic(vline, i)/mFontW, k);
            if (is_selected) {
                sel += Utf32ToUtf8(vline[i]);
            }
        }
        out.push_back(sel);
    }
    return out;
}
//
String SerialConnVT::GetSelectedText() const
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
bool SerialConnVT::IsCharInSelectionSpan(int x, int y) const
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
    int abs_x = x, abs_y = y;
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


int SerialConnVT::GetLongestLineSize(const std::vector<VTLine>& vlines, bool in_logic) const
{
    int max_sz = 0;
    for (size_t k = 0; k < vlines.size(); ++k) {
        int sz = (int)vlines[k].size() - this->CalculateNumberOfBlankCharsFromEnd(vlines[k]);
        if (in_logic) {
            int lx = 0;
            for (size_t i = 0; i < sz; ++i) {
                lx += GetCharWidth(vlines[k][i]);
            }
            sz = lx;
        }
        if (sz > max_sz) {
            max_sz = sz;
        }
    }
    return max_sz;
}

void SerialConnVT::Render(Upp::Draw& draw)
{
    // draw background
    draw.DrawRect(GetRect(), mPaperColor);
    // set total
    int nlines = (int)mLinesBuffer.size() + (int)mLines.size();
    mSbV.SetTotal(mFontH * nlines);
    // draw VT chars
    if (mScrollToEnd) { // draw current screen
        // the longest size
        int hsz = this->GetLongestLineSize(mLines);
        mSbH.SetTotal(hsz);
        draw.DrawRect(-mSbH.Get() + mCursorX*mFontW, mCursorY*mFontH, mFontW, mFontH, Color(0, 255, 0));
        mSbV.End(); // scroll to end
        // draw text
        Render(draw, mLines);
    } else { // draw buffered screen
        int y = -1;
        int p = mSbV / mFontH;
        auto vlines = GetBufferLines(p, y);
        if (y > 0) {
            draw.DrawRect(-mSbH.Get() + mCursorX*mFontW, y*mFontH, mFontW, mFontH, Color(0, 255, 0));
        }
        int hsz = this->GetLongestLineSize(vlines);
        mSbH.SetTotal(hsz);
        Render(draw, vlines);
    }
}

void SerialConnVT::Render(Upp::Draw& draw, const std::vector<VTLine>& vlines)
{
    Size csz = GetConsoleSize();
    Size usz = GetSize();
    int x = 0, y = 0, nlines = (int)vlines.size();
    // render all chars.
    for (size_t k = 0; k < nlines; ++k) {
        const VTLine& vline = vlines[k];
        int bx = mSbH.Get(); int vx = LogicToVirtual(vline, bx);
        int lx = VirtualToLogic(vline, vx); x = lx - bx;
        for (size_t i = vx; i < vline.size() && x < usz.cx; ++i) {
            vline[i].ApplyAttrs();
            // IsCharInSelectionSpan need absolute position of vchar
            bool is_selected = IsCharInSelectionSpan(lx/mFontW, mSbV.Get()/mFontH + (int)k);
            if (is_selected) {
                std::swap(mBgColor, mFgColor);
            }
            int vchar_cx = GetCharWidth((int)vline[i]); lx += vchar_cx;
            // To UTF8
            String buff = Utf32ToUtf8(vline[i]);
            if (mBlink) {
                if (mBlinkSignal) {
                    draw.DrawRect(x, y, vchar_cx, mFontH, mPaperColor);
                } else {
                    if (mBgColor != mDefaultBgColor) {
                        draw.DrawRect(x, y, vchar_cx, mFontH, mBgColor);
                    }
                    if (mFgColor != mBgColor) {
                        if (buff[0] != ' ' && mVisible) {
                            draw.DrawText(x, y, buff, mFont, mFgColor);
                        }
                    }
                }
            } else {
                if (mBgColor != mDefaultBgColor) {
                    draw.DrawRect(x, y, vchar_cx, mFontH, mBgColor);
                }
                if (mFgColor != mBgColor) {
                    if (buff[0] != ' ' && mVisible) {
                        draw.DrawText(x, y, buff, mFont, mFgColor);
                    }
                }
            }
            x += vchar_cx;
            if (is_selected) {
                if (usz.cx - x < mFontW) { // padding
                    draw.DrawRect(x, y, usz.cx-x, mFontH, mBgColor);
                }
                std::swap(mBgColor, mFgColor);
            }
        }
        // apply attrs for those lines longer than screen width
        for (size_t i = csz.cx; i < vline.size(); ++i) {
            vline[i].ApplyAttrs();
        }
        // move to next line
        y += mFontH;
    }
}
