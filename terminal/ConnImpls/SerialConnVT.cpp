//
// (c) 2020 chiv
//
#include "terminal_rc.h"
#include "SerialConnVT.h"
#include "ControlSeq.h"
#include "ConnFactory.h"
#include <algorithm>

REGISTER_CONN_INSTANCE("Original VT", SerialConnVT);
// register
using namespace Upp;
//
SerialConnVT::SerialConnVT(std::shared_ptr<SerialIo> serial)
    : SerialConn(serial)
    , mPx(0)
    , mPy(0)
    , mVx(0)
    , mVy(0)
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
    mFont = Monospace();
    // handlers
    mFontW = mFont.GetWidth('M');
    mFontH = mFont.GetLineHeight();
    // default style
    mCurrAttrFuncs.push_back([=]() { SetDefaultStyle(); });
    mBlankChar = ' ';
    mBlankChar.SetAttrFuns(mCurrAttrFuncs);
    // enable scroll bar
    mSbH.SetLine(mFontW); mSbH.Set(0); mSbV.SetTotal(0); AddFrame(mSbH);
    mSbH.WhenScroll = [=]() {
        Refresh();
    };
    // vertical
    mSbV.SetLine(mFontH); mSbV.Set(0); mSbV.SetTotal(0); AddFrame(mSbV);
    mSbV.WhenScroll = [=]() {
        int lposy = mSbV.Get();
        if (lposy + GetSize().cy >= mSbV.GetTotal()) {
            mScrollToEnd = true;
        } else {
            mScrollToEnd = false;
        }
        Refresh();
    };
    // blink timer
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
    mUsrActions.emplace_back(terminal::clear_buffer(), t_("Clear Buffer"),
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
    //
    mVx = 0;
    mVy = 0;
    mPx = 0;
    mPy = 0;
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
                        PostCallback([=]() {
                            ProcessControlSeq(pattern, ret);
                            if (ret != SEQ_NONE) {
                                ProcessOverflowLines();
                                UpdatePresentation();
                            }
                        });
                        pending = false;
                        pattern = "";
                    }
                } else if (buff[k] == 0x1b) {
                    // come across a 0x1b, render the raw
                    if (!raw.empty()) {
                        size_t ep; auto s = Utf8ToUtf32(raw, ep);
                        raw = raw.substr(ep);
                        PostCallback([=]() { RenderText(s); ProcessOverflowLines(); UpdatePresentation(); });
                    }
                    pending = true; // continue
                } else {
                    raw.push_back(buff[k]);
                }
            }
            if (!raw.empty()) {
                size_t ep; auto s = Utf8ToUtf32(raw, ep);
                raw = raw.substr(ep);
                PostCallback([=]() { RenderText(s); ProcessOverflowLines(); UpdatePresentation(); });
            }
            // callback is
            PostCallback([=]() { this->Refresh(); });
        } else std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
    }
}

int SerialConnVT::GetCharWidth(const VTChar& c) const
{
    if (c == '\t') {
        return mFontW * atoi(c.Var("CellSize").c_str());
    } else {
        int cx = mFont.GetWidth((int)c);
        // adjust width
        return (cx + mFontW-1) / mFontW * mFontW;
    }
}

int SerialConnVT::IsControlSeq(const std::string& seq)
{
    (void)seq;
    return SEQ_NONE;
}

Size SerialConnVT::GetConsoleSize() const
{
    Size sz = GetSize();
    if (sz.cx <= 0 || sz.cy <= 0) {
        int cx = mLines.empty() ? 0 : (int)mLines[0].size();
        int cy = (int)mLines.size();
            return Size(cx, cy);
    }
    return Size(sz.cx / mFontW, sz.cy / mFontH);
}
// lx, ly - lines is virtual screen
Point SerialConnVT::LogicToVirtual(const std::vector<VTLine>& lines, int lx, int ly) const
{
    // find vy
    int vy = -1, py = 0;
    for (int i = 0; i < (int)lines.size(); ++i) {
        py += lines[i].GetHeight();
        if (py >= ly) {
            vy = i;
            break;
        }
    }
    if (vy < 0)
        return Point(-1, -1);
    int px = 0, vx = 0;
    for (int i = 0; i < (int)lines[vy].size(); ++i) {
        px += GetCharWidth(lines[vy][i]);
        if (px >= lx) {
            vx = i;
            break;
        }
    }
    return Point(vx, vy);
}
//
Point SerialConnVT::VirtualToLogic(const std::vector<VTLine>& lines, int vx, int vy) const
{
    if (vy < 0 || vy >= (int)lines.size())
        return Size(-1, -1);
    int ly = 0;
    for (size_t k = 0; k < vy; ++k) {
        ly += lines[k].GetHeight();
    }
    int lx = 0;
    for (size_t k = 0; k < vx; ++k) {
        lx += GetCharWidth(lines[vy][k]);
    }
    return Point(lx, ly);
}
//
Point SerialConnVT::VirtualToLogic(int vx, int vy)  const
{
    const VTLine* vline = nullptr;
    if (vy < 0 || vy >= (int)(mLines.size() + mLinesBuffer.size()))
        return Size(-1, -1);
    if (vy < (int)mLinesBuffer.size()) {
        vline = &mLinesBuffer[vy];
    } else {
        vline = &mLines[vy - (int)mLinesBuffer.size()];
    }
    int ly = 0;
    for (size_t k = 0; k < (int)mLinesBuffer.size() && k < vy; ++k) {
        ly += mLinesBuffer[k].GetHeight();
    }
    for (int k = (int)mLinesBuffer.size(); k < vy; ++k) {
        ly += mLines[k - (int)mLinesBuffer.size()].GetHeight();
    }
    //
    int lx = 0;
    for (size_t k = 0; k < vx; ++k) {
        lx += GetCharWidth(vline->at(k));
    }
    return Point(lx, ly);
}
//
Point SerialConnVT::LogicToVirtual(int lx, int ly) const
{
    int vy = -1, py = 0;
    for (int i = 0; i < (int)mLinesBuffer.size(); ++i) {
        py += mLinesBuffer[i].GetHeight();
        if (py > ly) {
            vy = i;
            break;
        }
    }
    if (vy < 0) {
        for (int i = 0; i < (int)mLines.size(); ++i) {
            py += mLines[i].GetHeight();
            if (py > ly) {
                vy = (int)mLinesBuffer.size() + i;
                break;
            }
        }
    }
    if (vy < 0)
        return Point(-1, -1);
    const VTLine* vline = nullptr;
    if (vy < (int)mLinesBuffer.size()) {
        vline = &mLinesBuffer[vy];
    } else {
        vline = &mLines[vy - (int)mLinesBuffer.size()];
    }
    int px = 0, vx = 0;
    for (int k = 0; k < (int)vline->size(); ++k) {
        px += GetCharWidth(vline->at(k));
        if (px >= lx) {
            vx = k;
            break;
        }
    }
    return Point(vx, vy);
}
//
int SerialConnVT::LogicToVirtual(const VTLine& vline, int lx) const
{
    int px = 0, vx = 0;
    for (int i = 0; i < (int)vline.size(); ++i) {
        px += GetCharWidth(vline[i]);
        if (px > lx) {
            vx = i;
            break;
        }
    }
    return vx;
}
//
int SerialConnVT::VirtualToLogic(const VTLine& vline, int vx) const
{
    int lx = 0;
    for (int i = 0; i < vx; ++i) {
        lx += GetCharWidth(vline[i]);
    }
    return lx;
}
//
int SerialConnVT::GetLogicWidth(const VTLine& vline, int count) const
{
    int x = 0;
    if (count < 0) count = (int)vline.size();
    for (int k = 0; k < count; ++k) {
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
bool SerialConnVT::ProcessAsciiControlChar(char cc)
{
    switch (cc) {
    case '\v':
        mLines[mVy][mVx++] = '\v';
        mVy++;
        break;
    case '\b':
        if (mVx > 0) {
            mVx--;
        }
        break;
    case '\r':
        mVx = 0;
        break;
    case '\n':
        mVy += 1;
        break;
    case '\t': if (1) {
        int cellsz = mVx/4*4 + 4 - mVx;
        mLines[mVy][mVx] = '\t';
        mLines[mVy][mVx].SetVar("CellSize", std::to_string(cellsz));
        mVx++;
    } break;
    }
    return true;
}
//
void SerialConnVT::ProcessControlSeq(const std::string& seq, int seq_type)
{
    if (seq_type == SEQ_NONE) { // can't recognize the seq, display it
        size_t ep;RenderText(Utf8ToUtf32(seq, ep));
    }
}
//
void SerialConnVT::ProcessOverflowLines()
{
    Size csz = GetConsoleSize();
    if (mVy >= csz.cy) { // scroll
        size_t cn = mVy - csz.cy + 1;
        for (size_t k = 0; k < cn; ++k) {
            PushToLinesBufferAndCheck(*mLines.begin());
            mLines.erase(mLines.begin());
            mLines.push_back(VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
        }
        mVy = csz.cy - 1; // move to last line
    }
}
void SerialConnVT::RenderText(const std::vector<uint32_t>& s)
{
    if (mVy >= (int)mLines.size()) return;
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
            VTLine& vline = mLines[mVy];
            if (mVx < vline.size()) {
                vline[mVx] = chr;
            } else {
                // padding blanks if needed.
                for (int n = (int)vline.size()-1; n < mVx-1; ++n) {
                    vline.push_back(mBlankChar);
                }
                // extend the vline
                vline.push_back(chr);
            }
            mVx++;
        }
    }
}

Image SerialConnVT::CursorImage(Point p, dword keyflags)
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

int SerialConnVT::GetLineHeight(int vy) const
{
    int height = 0;
    if (vy >= 0) {
        if (vy < (int)mLinesBuffer.size()) {
            height = mLinesBuffer[vy].GetHeight();
        } else if (vy - (int)mLinesBuffer.size() < (int)mLines.size()) {
            height = mLines[vy - (int)mLinesBuffer.size()].GetHeight();
        }
    }
    return height;
}

void SerialConnVT::MouseMove(Point p, dword)
{
    if (mPressed) {
        if (p.x - mSelectionSpan.x0 == 0 && p.y - mSelectionSpan.y0 == 0) {
            return; // No movement
        }
        Point vpos = LogicToVirtual(mSbH.Get() + p.x, mSbV.Get() + p.y);
        mSelectionSpan.X1 = vpos.x;
        mSelectionSpan.Y1 = vpos.y;
        // scroll over
        if (p.y < 0) {
            int b0 = mSbV.Get();
            mSbV.Set(mSbV.Get() - this->GetLineHeight(vpos.y - 1));
            Rect rc = this->GetCaret();
            rc.Offset(0, -mSbV.Get() + b0);
            SetCaret(rc);
        } else if (p.y > GetSize().cy) {
            int b0 = mSbV.Get();
            mSbV.Set(mSbV.Get() + this->GetLineHeight(vpos.y + 1));
            Rect rc = this->GetCaret();
            rc.Offset(0, mSbV.Get() - b0);
            SetCaret(rc);
        }
        if (p.x < 0) {
            int b0 = mSbV.Get();
            mSbH.Set(mSbH.Get() - this->GetLineHeight(vpos.y - 1));
            Rect rc = this->GetCaret();
            rc.Offset(0, -mSbH.Get() + b0);
            SetCaret(rc);
        } else if (p.x > GetSize().cx) {
            int b0 = mSbV.Get();
            mSbH.Set(mSbH.Get() + this->GetLineHeight(vpos.y + 1));
            Rect rc = this->GetCaret();
            rc.Offset(0, mSbH.Get() - b0);
            SetCaret(rc);
        }
        //
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
    mPressed = true;
    Point vpos = this->LogicToVirtual(mSbH.Get() + p.x, mSbV.Get() + p.y);
    Point lpos = this->VirtualToLogic(vpos.x, vpos.y);
    
    mSelectionSpan.X0 = vpos.x;
    mSelectionSpan.X1 = vpos.x;
    mSelectionSpan.Y0 = vpos.y;
    mSelectionSpan.Y1 = vpos.y;
    mSelectionSpan.x0 = mSelectionSpan.x1 = p.x;
    mSelectionSpan.y0 = mSelectionSpan.y1 = p.y;
    //
    int caret_height = GetLineHeight(vpos.y);
    this->SetCaret(lpos.x - mSbH.Get(), lpos.y - mSbV.Get(), 1, caret_height);
    // capture, event the cursor move out of the client region.
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

void SerialConnVT::Paste()
{
    String text = ReadClipboardUnicodeText().ToString();
    GetSerial()->Write(text.ToStd());
}

void SerialConnVT::Copy()
{
    WriteClipboardText(GetSelectedText());
}

void SerialConnVT::SelectAll()
{
    if (!mLinesBuffer.empty() || !mLines.empty()) {
        mSelectionSpan.X0 = 0;
        mSelectionSpan.X1 = 0;
        mSelectionSpan.Y0 = 0;
        mSelectionSpan.Y1 = (int)(mLinesBuffer.size() + mLines.size());
        //
        Refresh();
    }
}

void SerialConnVT::RightUp(Point, dword)
{
	MenuBar::Execute([=](Bar& bar) {
	    bool has_sel = mSelectionSpan.X0 != mSelectionSpan.X1 ||
	                   mSelectionSpan.Y0 != mSelectionSpan.Y1;
		bar.Add(has_sel, t_("Copy"), [=] {
		    // copy to clipboard
		    Copy();
		}).Image(CtrlImg::copy()).Key(K_CTRL | K_SHIFT | K_C);
		String text = ReadClipboardUnicodeText().ToString();
		bar.Add(text.GetCount() > 0, t_("Paste"), [=] {
		    Paste();
		}).Image(CtrlImg::paste()).Key(K_CTRL | K_SHIFT | K_V);
		bar.Add(true, t_("Select all"), [=]() {
		    SelectAll();
		}).Key(K_CTRL | K_SHIFT | K_A);
		//
		bar.Add("Test height", [=]() {
		    mLines[5].SetHeight(25);
		    UpdatePresentation();
		    Refresh();
		});
	});
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
    if ((flags & (K_CTRL | K_SHIFT)) == (K_CTRL | K_SHIFT)) { // CTRL+SHIFT+
        switch (key) {
        case K_C: Copy(); return true;
        case K_A: SelectAll(); return true;
        case K_V: Paste(); return true;
            break;
        }
    }
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
	return processed;
}
//
int SerialConnVT::CalculateNumberOfBlankLinesFromEnd(const std::vector<VTLine>& lines) const
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
	        vline.push_back(mBlankChar);
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
	            vline.push_back(mBlankChar);
	        }
	        mLines.insert(mLines.begin(), vline);
	        ln++;
	    }
	    mVy += ln;
	    // 1. push blanks
	    for (int i = ln; i < csz.cy && i < extcn; ++i) {
	        mLines.push_back(VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
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
	        if (mVy > 0) {
	            mVy--;
	        }
	    }
	}
}

void SerialConnVT::Layout()
{
	DoLayout();
	//
	UpdatePresentation();
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
        int vx = span.X0;
        for (size_t i = vx; i < nchars; ++i) {
            bool is_selected = IsCharInSelectionSpan(i, span.Y0);
            if (is_selected) {
                sel += Utf32ToUtf8(vline[i]);
            }
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
        int vx = span.X1;
        for (size_t i = 0; i < nchars && i < vx; ++i) {
            bool is_selected = IsCharInSelectionSpan(i, span.Y1);
            if (is_selected) {
                sel += Utf32ToUtf8(vline[i]);
            }
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
        // strip tail blanks of lines or \v
        size_t n = 0;
        while (n < lines[k].size()) {
            uint32_t c = lines[k][lines[k].size()-1-n];
            if (c == ' ' || c == '\n' || c == '\v')
                n++;
            else break;
        }
        out += lines[k].substr(0, lines[k].size() - n);
    }
    return out;
}
//
bool SerialConnVT::IsCharInSelectionSpan(int vx, int vy) const
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
    int abs_x = vx, abs_y = vy;
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

void SerialConnVT::DrawCursor(Draw& draw)
{
    draw.DrawRect(mPx, mPy, mFontW, mFontH, Color(0, 255, 0));
}

int SerialConnVT::GetVTLinesHeight(const std::vector<VTLine>& lines) const
{
    int linesz = 0;
    for (size_t k = 0; k < lines.size(); ++k) {
        linesz += lines[k].GetHeight();
    }
    return linesz;
}

void SerialConnVT::DrawVTLine(Draw& draw, const VTLine& vline,
                              int vx, int vy, /*! absolute position of data */
                              int lxoff, int lyoff)
{
    Size usz = GetSize(), csz = GetConsoleSize();
    int x = lxoff;
    int y = lyoff;
    for (int i = vx; i < (int)vline.size() && x < usz.cx; ++i) {
        vline[i].ApplyAttrs();
        bool is_selected = IsCharInSelectionSpan(i, vy);
        if (is_selected) {
            std::swap(mBgColor, mFgColor);
        }
        int vchar_cx = GetCharWidth(vline[i]);
        if (mBlink) {
            if (mBlinkSignal) {
                draw.DrawRect(x, y, vchar_cx, vline.GetHeight(), mPaperColor);
            } else {
                if (mBgColor != mDefaultBgColor) {
                    draw.DrawRect(x, y, vchar_cx, vline.GetHeight(), mBgColor);
                }
                if (mFgColor != mBgColor) {
                    if (vline[i] != ' ' && vline[i] != '\t' && vline[i] != '\v' && mVisible) {
                        DrawVTChar(draw, x, y, vline[i], mFont, mFgColor);
                    }
                }
            }
        } else {
            if (mBgColor != mDefaultBgColor) {
                draw.DrawRect(x, y, vchar_cx, vline.GetHeight(), mBgColor);
            }
            if (mFgColor != mBgColor) {
                if (vline[i] != ' ' && vline[i] != '\t' && vline[i] != '\v' && mVisible) {
                    DrawVTChar(draw, x, y, vline[i], mFont, mFgColor);
                }
            }
        }
        x += vchar_cx;
        if (is_selected) {
            if (usz.cx - x < mFontW) { // padding
                draw.DrawRect(x, y, usz.cx-x, vline.GetHeight(), mBgColor);
            }
            std::swap(mBgColor, mFgColor);
        }
        
    }
    for (int i = csz.cx; i < (int)vline.size(); ++i) {
        vline[i].ApplyAttrs();
    }
}

void SerialConnVT::UpdatePresentationPos()
{
    Point ppos = VirtualToLogic(mVx, (int)mLinesBuffer.size() + mVy);
    mPx = ppos.x - mSbH.Get();
    mPy = ppos.y - mSbV.Get();
}

void SerialConnVT::UpdateScrollbar()
{
    mSbV.SetTotal((int)(GetVTLinesHeight(mLines) + GetVTLinesHeight(mLinesBuffer)));
    if (mScrollToEnd) {
        mSbV.End();
    }
    Point vpos = LogicToVirtual(0, mSbV.Get());
    if (vpos.x < 0 || vpos.y < 0) return;
    // let's begin
    Size usz = GetSize(); int vy = vpos.y;
    int longest_linesz = 0, lyoff = 0;
    for (int i = vpos.y; i < (int)mLinesBuffer.size(); ++i) {
        const VTLine& vline = mLinesBuffer[i];
        lyoff += vline.GetHeight();
        int linesz = this->GetLogicWidth(vline, (int)vline.size() - this->CalculateNumberOfBlankCharsFromEnd(vline));
        if (linesz > longest_linesz) {
            longest_linesz = linesz;
        }
    }
    for (int i = 0; i < (int)mLines.size(); ++i) {
        const VTLine& vline = mLines[i];
        lyoff += vline.GetHeight();
        if (lyoff >= usz.cy)
            break;
        int linesz = this->GetLogicWidth(vline, (int)vline.size() - this->CalculateNumberOfBlankCharsFromEnd(vline));
        if (linesz > longest_linesz) {
            longest_linesz = linesz;
        }
    }
    mSbH.SetTotal(longest_linesz);
}

void SerialConnVT::UpdatePresentation()
{
    UpdateScrollbar();
    UpdatePresentationPos();
}

void SerialConnVT::DrawVT(Draw& draw)
{
    // calculate offset
    Point vpos = LogicToVirtual(mSbH.Get(), mSbV.Get());
    if (vpos.x < 0 || vpos.y < 0) return;
    Point lpos = VirtualToLogic(vpos.x, vpos.y);
    int lxoff = lpos.x - mSbH.Get();
    int lyoff = lpos.y - mSbV.Get();
    //--------------------------------------------------------------
    // draw lines, and calculate the presentation information
    Size usz = GetSize(); int vy = vpos.y;
    for (int i = vpos.y; i < (int)mLinesBuffer.size(); ++i) {
        const VTLine& vline = mLinesBuffer[i];
        DrawVTLine(draw, vline, vpos.x, vy++, lxoff, lyoff);
        lyoff += vline.GetHeight();
    }
    for (int i = std::max(0, vpos.y - (int)mLinesBuffer.size()); i < (int)mLines.size(); ++i) {
        const VTLine& vline = mLines[i];
        DrawVTLine(draw, vline, vpos.x, vy++, lxoff, lyoff);
        lyoff += vline.GetHeight();
        if (lyoff >= usz.cy)
            break;
    }
}

void SerialConnVT::Render(Draw& draw)
{
    // draw background
    draw.DrawRect(GetRect(), mPaperColor);
    //
    DrawVT(draw);
    //
    UpdatePresentationPos();
    //
    DrawCursor(draw);
}

std::string SerialConnVT::TranscodeToUTF8(const VTChar& cc) const
{
    return Utf32ToUtf8(cc);
}

void SerialConnVT::DrawVTChar(Draw& draw, int x, int y, const VTChar& c,
                              const Font& font, const Color& cr)
{
    // To UTF8
    std::string text = TranscodeToUTF8(c);
    draw.DrawText(x, y, text, font, cr);
}

void SerialConnVT::Paint(Draw& draw)
{
    Render(draw);
}
