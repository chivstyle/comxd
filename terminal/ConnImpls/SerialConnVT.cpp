//
// (c) 2020 chiv
//
#include "resource.h"
#include "SerialConnVT.h"
#include "ControlSeq.h"
#include "ConnFactory.h"
#include <algorithm>
// register
using namespace Upp;
//
SerialConnVT::SerialConnVT(std::shared_ptr<SerialIo> serial)
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
    // double buffer
    BackPaint();
    // default font
    mFont = Upp::Monospace();
    // handlers
    mFontW = mFont.GetWidth('M');
    mFontH = mFont.GetLineHeight();
    // default style
    mCurrAttrFuncs.push_back([=]() { SetDefaultStyle(); });
    mBlankChr = ' ';
    mBlankChr.SetAttrFuns(mCurrAttrFuncs);
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
    SetTimeCallback(-500, [&]() {
        mBlinkSignal = !mBlinkSignal;
        this->Refresh();
        }, kBlinkTimerId);
    //
    mRxThr = std::thread([=]() { RxProc(); });
    //
    InstallUserActions();
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
void SerialConnVT::InstallUserActions()
{
    mUsrActions.emplace_back(comxd::clear_buffer(), t_("Clear Buffer"),
        t_("Clear the line buffers"), [=]() { Clear(); }
    );
}
//
void SerialConnVT::SetDefaultStyle()
{
    mFont.NoBold().NoItalic().NoUnderline();
    mBgColor = mDefaultBgColor;
    mFgColor = mDefaultFgColor;
    mBlink = false;
}
//
void SerialConnVT::Clear()
{
    SetDefaultStyle();
    mLinesBuffer.clear();
    mLines.clear();
    DoLayout();
    mCursorX = 0;
    mCursorY = 0;
    //
    Refresh();
}
// receiver
void SerialConnVT::RxProc()
{
    bool pending = false; // If pending is true, that stands for a VT seq is pending
                          // we should treat the successive characters as a part of last
                          // VT seq.
    std::string pattern, raw;
    while (!mRxShouldStop) {
        size_t sz = mSerial->Available();
        if (sz) {
            std::string buff = GetSerial()->Read(sz);
            for (size_t k = 0; k < sz; ++k) {
                if (pending) {
                    pattern.push_back((char)buff[k]);
                    int ret = IsControlSeq(pattern);
                    if (ret != SEQ_PENDING) { // bingo
                        // found it.
                        Upp::PostCallback([=]() { ProcessControlSeq(pattern, ret); ProcessOverflowLines(); });
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

int SerialConnVT::GetCharWidth(const VTChar& c) const
{
    int cx = mFont.GetWidth((int)c);
    // adjust width
    return (cx + mFontW-1) / mFontW * mFontW;
}

Size SerialConnVT::GetConsoleSize() const
{
    Size sz = GetSize();
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
void SerialConnVT::ProcessAsciiControlChar(char cc)
{
    // only support CR,LF in VT.
    if (cc == '\r') {
        mCursorX = 0;
    } else if (cc == '\n') {
        mCursorY += 1;
    } else {
        size_t ep;
        char buf[32]; sprintf(buf, "\\x%02x", cc);
        RenderText(Utf8ToUtf32(buf, ep));
    }
}
//
void SerialConnVT::ProcessControlSeq(const std::string& seq, int seq_type)
{
    if (seq_type == SEQ_NONE) { // can't recognize the seq, display it
        size_t ep;RenderText(Utf8ToUtf32(seq, ep));
    } else {
        // Never reach here, if your terminal was designed well.
    }
}
//
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
    if (mCursorY >= (int)mLines.size()) return;
    Size csz = GetConsoleSize();
    for (size_t k = 0; k < s.size(); ++k) {
        VTChar chr = s[k];
        if (chr < 0x20 || chr == 0x7f) {
            ProcessAsciiControlChar(chr);
            ProcessOverflowLines();
        } else {
            chr.SetAttrFuns(mCurrAttrFuncs);
            // Unfortunately, UPP does not support complete UNICODE, support UCS-16 instead. So
            // we should ignore those out of range
            if (chr > 0xffff) chr = 0x25A1; // replace chr with U+25A1 ( □ )
            VTLine& vline = mLines[mCursorY];
            if (mCursorX < vline.size()) {
                vline[mCursorX] = chr;
            } else {
                // padding blanks if needed.
                for (int n = (int)vline.size()-1; n < mCursorX-1; ++n) {
                    vline.push_back(mBlankChr);
                }
                // extend the vline
                vline.push_back(chr);
            }
            mCursorX++;
        }
    }
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
        if (p.x - mSelectionSpan.x0 == 0 && p.y - mSelectionSpan.y0 == 0) {
            return; // No movement
        }
        int nlines = (int)mLinesBuffer.size() + \
            (int)mLines.size() - this->CalculateNumberOfBlankLinesFromEnd(mLines);
        mSelectionSpan.X1 = (mSbH.Get() + p.x + mFontW-1) / mFontW;
        //
        int y = (mSbV.Get() + p.y) / mFontH; // absolute position
        if (y >= nlines) {
            mSelectionSpan.Y1 = nlines - 1;
        } else {
            mSelectionSpan.Y1 = y;
        }
        if (p.y < 0) {
            int b0 = mSbV.Get();
            mSbV.PrevLine();
            Rect rc = this->GetCaret();
            rc.Offset(0, -mSbV.Get() + b0);
            SetCaret(rc);
        } else if (p.y > GetSize().cy) {
            int b0 = mSbV.Get();
            mSbV.NextLine();
            Rect rc = this->GetCaret();
            rc.Offset(0, mSbV.Get() - b0);
            SetCaret(rc);
        }
        if (p.x < 0) {
            int b0 = mSbV.Get();
            mSbH.PrevLine();
            Rect rc = this->GetCaret();
            rc.Offset(0, -mSbH.Get() + b0);
            SetCaret(rc);
        } else if (p.x > GetSize().cx) {
            int b0 = mSbV.Get();
            mSbH.NextLine();
            Rect rc = this->GetCaret();
            rc.Offset(0, mSbH.Get() - b0);
            SetCaret(rc);
        }
        mSelectionSpan.x1 = p.x;
        mSelectionSpan.y1 = p.y;
        // fix it
        if (mSelectionSpan.X0 < 0) mSelectionSpan.X0 = 0;
        if (mSelectionSpan.X1 < 0) mSelectionSpan.X1 = 0;
        if (mSelectionSpan.Y0 < 0) mSelectionSpan.Y0 = 0;
        if (mSelectionSpan.Y1 < 0) mSelectionSpan.Y1 = 0;
        //
        Refresh();
    }
}

void SerialConnVT::LeftDown(Point p, dword)
{
    int nlines = (int)mLinesBuffer.size() + \
        (int)mLines.size() - this->CalculateNumberOfBlankLinesFromEnd(mLines);
    mPressed = true;
    mSelectionSpan.Y0 = (mSbV.Get() + p.y) / mFontH;
    mSelectionSpan.Y1 = (mSbV.Get() + p.y) / mFontH;
    mSelectionSpan.x0 = mSelectionSpan.x1 = p.x;
    mSelectionSpan.y0 = mSelectionSpan.y1 = p.y;
    int lx = 0;
    if (mSelectionSpan.Y0 >= mLinesBuffer.size()) {
        int ln = mSelectionSpan.Y0 - (int)mLinesBuffer.size();
        if (ln < (int)mLines.size()) {
            lx = VirtualToLogic(mLines[ln], LogicToVirtual(mLines[ln], mSbH.Get() + p.x));
        }
    } else {
        lx = VirtualToLogic(mLinesBuffer[mSelectionSpan.Y0], LogicToVirtual(mLinesBuffer[mSelectionSpan.Y0], mSbH.Get() + p.x));
    }
    mSelectionSpan.X0 = mSelectionSpan.X1 = lx / mFontW;
    this->SetCaret(lx - mSbH.Get(), p.y/mFontH*mFontH, 1, mFontH);
    //
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

void SerialConnVT::LeftUp(Point p, dword)
{
    (void)p;
    if (mPressed) {
        this->KillCaret();
        mPressed = false;
        Refresh();
    }
}

void SerialConnVT::RightUp(Point, dword)
{
	MenuBar::Execute(
		[=](Bar& bar) {
		    bool has_sel = mSelectionSpan.X0 != mSelectionSpan.X1 ||
		                   mSelectionSpan.Y0 != mSelectionSpan.Y1;
			bar.Add(has_sel, t_("Copy"), [=] {
			    // copy to clipboard
			    Upp::WriteClipboardText(GetSelectedText());
			});
			String text2 = Upp::ReadClipboardUnicodeText().ToString();
			bar.Add(text2.GetCount() > 0, t_("Paste"), [=] {
			    GetSerial()->Write(text2.ToStd());
			});
		}
	);
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
    } else if ((flags & (K_CTRL | K_SHIFT)) == (K_CTRL | K_SHIFT)) { // CTRL+SHIFT+
        switch (key) {
        case K_2:
            d.push_back(0x00); // NUL
            break;
        case K_6:
            d.push_back(0x1e); // RS
            break;
        case K_MINUS:
            d.push_back(0x1f); // US
            break;
        }
    } else if ((flags & K_CTRL) == K_CTRL) {
        if (key >= K_A && key <= K_Z) {
            d.push_back(1 + key - K_A);
        } else {
            switch (key) {
            case K_LBRACKET: // '[':
                d.push_back(0x1b); // ESC
                break;
            case K_SLASH: // '/':
                d.push_back(0x1c); // FS
                break;
            case K_RBRACKET: // ']':
                d.push_back(0x1d); // GS
                break;
            }
        }
    }
    if (!d.empty()) {
        GetSerial()->Write(d);
        return true;
    }
    return false;
}

bool SerialConnVT::ProcessKeyUp(dword key, dword flags)
{
    return false;
}

bool SerialConnVT::ProcessChar(dword cc)
{
    GetSerial()->Write(Utf32ToUtf8(cc));
    //
    return true;
}

bool SerialConnVT::Key(dword key, int)
{
    bool processed = false;
    // split key and flags.
	dword flags = K_CTRL | K_ALT | K_SHIFT;
    dword d_key = key & ~(flags | K_KEYUP); // key with delta
    flags = key & flags;
    //
	if (d_key & K_DELTA) { // can't capture RETURN on Windows/Linux
	    if (key & K_KEYUP) {
	        processed = ProcessKeyUp(d_key, flags);
	    } else {
	        processed = ProcessKeyDown(d_key, flags);
	    }
	} else {
	    if (key < 0xffff) {
	        processed = ProcessChar(d_key);
	        // Windows/Linux
	        if (d_key == 0x0d) {
	            ProcessKeyDown(d_key, flags);
	        }
	    } else if (key & K_KEYUP) {
	        // RETURN will reach here on Windows/Linux
	        ProcessKeyUp(d_key, flags);
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
//
void SerialConnVT::DoLayout()
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
}

void SerialConnVT::Layout()
{
	DoLayout();
}

std::vector<SerialConnVT::VTLine> SerialConnVT::GetMergedScreen(size_t p, int& nlines_from_buffer) const
{
    Size csz = GetConsoleSize();
    std::vector<VTLine> out(csz.cy);
    // fetch lines from lines buffer
    size_t ln = 0;
    for (size_t k = p; k < mLinesBuffer.size() && (int)ln < csz.cy; ++k) {
        out[ln] = mLinesBuffer[k];
        for (int i = out[ln].size(); i < csz.cx; ++i) {
            out[ln].push_back(mBlankChr);
        }
        ln++;
    }
    nlines_from_buffer = ln;
    // padding, lines from Lines.
    int cn = 0;
    for (size_t k = ln; (int)k < csz.cy; ++k) {
        out[ln++] = mLines[cn++];
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
    if (span.X0 == span.X1 && span.Y0 == span.Y1) return std::vector<std::string>();
    // merge buffer and virtual screen.
    int nlines = (int)mLines.size() - this->CalculateNumberOfBlankLinesFromEnd(mLines) + (int)mLinesBuffer.size();
    std::vector<std::string> out(span.Y1-span.Y0 + 1); size_t out_p = 0;
    // first line
    if (1) {
        const VTLine& vline = span.Y0 >= (int)mLinesBuffer.size() ? mLines[span.Y0-(int)mLinesBuffer.size()] : mLinesBuffer[span.Y0];
        std::string sel;
        int nchars = (int)vline.size() - this->CalculateNumberOfBlankCharsFromEnd(vline);
        int lx = span.X0*mFontW;
        int vx = LogicToVirtual(vline, lx);
        for (size_t i = vx; i < nchars; ++i) {
            bool is_selected = IsCharInSelectionSpan(lx / mFontW, span.Y0);
            if (is_selected) {
                sel += Utf32ToUtf8(vline[i]);
            }
            lx += this->GetCharWidth(vline[i]);
        }
        out[out_p++] = sel;
    }
    // in span
    //------------------------------------------------------------------------------------------
    for (int k = span.Y0+1; k < span.Y1 && k < nlines; ++k) {
        const VTLine& vline = k >= (int)mLinesBuffer.size() ? mLines[k-(int)mLinesBuffer.size()] : mLinesBuffer[k];
        std::string sel;
        int nchars = (int)vline.size() - this->CalculateNumberOfBlankCharsFromEnd(vline);
        for (size_t i = 0; i < nchars; ++i) {
            sel += Utf32ToUtf8(vline[i]);
        }
        out[out_p++] = sel;
    }
    // tail line, if existed
    if (span.Y1 > span.Y0 && span.Y1 < nlines) {
        const VTLine& vline = span.Y1 >= (int)mLinesBuffer.size() ? mLines[span.Y1-(int)mLinesBuffer.size()] : mLinesBuffer[span.Y1];
        std::string sel;
        int nchars = (int)vline.size() - this->CalculateNumberOfBlankCharsFromEnd(vline);
        int lx = 0, max_lx = span.X1*mFontW;
        for (size_t i = 0; i < nchars && lx < max_lx; ++i) {
            bool is_selected = IsCharInSelectionSpan(lx / mFontW, span.Y1);
            if (is_selected) {
                sel += Utf32ToUtf8(vline[i]);
            }
            lx += this->GetCharWidth(vline[i]);
        }
        out[out_p++] = sel;
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
            if (abs_x >= span.X0 && abs_x < span.X1) return true;
        } else {
            return abs_x >= span.X0;
        }
    } else if (abs_y == span.Y1) {
        if (abs_x < span.X1) return true;
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

void SerialConnVT::DrawCursor(Upp::Draw& draw, int vx, int vy)
{
    int x = -mSbH.Get() + mFontW*vx;
    int y = mFontH*vy;
    if (y < GetSize().cy && y >= 0) {
        draw.DrawRect(x, y, mFontW, mFontH, Color(0, 255, 0));
    }
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
        DrawCursor(draw, mCursorX, mCursorY);
        mSbV.End(); // scroll to end
        // draw text
        DrawVLines(draw, mLines);
    } else { // draw buffered screen
        int y = -1;
        int p = mSbV / mFontH;
        auto vlines = GetMergedScreen(p, y);
        y += mCursorY;
        DrawCursor(draw, mCursorX, y);
        //
        int hsz = this->GetLongestLineSize(vlines);
        mSbH.SetTotal(hsz);
        DrawVLines(draw, vlines);
    }
}

void SerialConnVT::DrawText(Upp::Draw& draw, int x, int y, const std::string& text,
    const Upp::Font& font, const Upp::Color& cr)
{
    draw.DrawText(x, y, text, font, cr);
}

std::string SerialConnVT::TranscodeToUTF8(const VTChar& cc) const
{
    return Utf32ToUtf8(cc);
}

void SerialConnVT::DrawVLine(Draw& draw, int vx, int vy, const VTLine& vline)
{
    Size usz = GetSize();
    int lx = VirtualToLogic(vline, vx);
    int x = lx - mSbH.Get(), y = vy * mFontH;
    for (size_t i = vx; i < vline.size() && x < usz.cx; ++i) {
        vline[i].ApplyAttrs();
        // IsCharInSelectionSpan need absolute position of vchar
        bool is_selected = IsCharInSelectionSpan(lx/mFontW, mSbV.Get()/mFontH + vy);
        if (is_selected) {
            std::swap(mBgColor, mFgColor);
        }
        int vchar_cx = GetCharWidth((int)vline[i]); lx += vchar_cx;
        // To UTF8
        std::string buff = TranscodeToUTF8(vline[i]);
        if (mBlink) {
            if (mBlinkSignal) {
                draw.DrawRect(x, y, vchar_cx, mFontH, mPaperColor);
            } else {
                if (mBgColor != mDefaultBgColor) {
                    draw.DrawRect(x, y, vchar_cx, mFontH, mBgColor);
                }
                if (mFgColor != mBgColor) {
                    if (buff[0] != ' ' && mVisible) {
                        DrawText(draw, x, y, buff, mFont, mFgColor);
                    }
                }
            }
        } else {
            if (mBgColor != mDefaultBgColor) {
                draw.DrawRect(x, y, vchar_cx, mFontH, mBgColor);
            }
            if (mFgColor != mBgColor) {
                if (buff[0] != ' ' && mVisible) {
                    DrawText(draw, x, y, buff, mFont, mFgColor);
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
}

void SerialConnVT::DrawVLines(Upp::Draw& draw, const std::vector<VTLine>& vlines)
{
    Size csz = GetConsoleSize();
    int x = 0, y = 0, nlines = (int)vlines.size();
    // render all chars.
    for (size_t k = 0; k < nlines; ++k) {
        const VTLine& vline = vlines[k];
        int vx = LogicToVirtual(vline, mSbH.Get());
        DrawVLine(draw, vx, (int)k, vline);
        // apply attrs for those lines longer than screen width
        for (size_t i = csz.cx; i < vline.size(); ++i) {
            vline[i].ApplyAttrs();
        }
    }
}

void SerialConnVT::Paint(Upp::Draw& draw)
{
    Render(draw);
}
