//
// (c) 2020 chiv
//
#include "terminal_rc.h"
#include "SerialConnVT.h"
#include "VTOptionsDialog.h"
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
    mFont = VTOptionsDialog::DefaultFont();
    // handlers
    mFontW = mFont.GetAveWidth();
    mFontH = mFont.GetLineHeight();
    // default style
    mCurrAttrFuncs.push_back([=]() { SetDefaultStyle(); });
    mBlankChar = ' ';
    mBlankChar.SetAttrFuns(mCurrAttrFuncs);
    // enable scroll bar
    mSbH.SetLine(mFontW); mSbH.Set(0); mSbV.SetTotal(0); AddFrame(mSbH);
    mSbH.WhenScroll = [=]() {
        UpdatePresentationPos();
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
        UpdateHScrollbar();
        UpdatePresentationPos();
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
    mUsrActions.emplace_back(terminal::clear_buffer(),
        t_("Clear Buffer"), t_("Clear the line buffers"), [=]() { Clear(); }
    );
    mUsrActions.emplace_back(terminal::vt_options(),
        t_("VT options"), t_("Virtual terminal options"), [=]() {
            VTOptionsDialog::Options options;
            options.Font = mFont;
            options.LinesBufferSize = this->mMaxLinesBufferSize;
            options.PaperColor = mPaperColor;
            options.FontColor = mDefaultFgColor;
            VTOptionsDialog opt; opt.SetOptions(options);
            int ret = opt.Run();
            if (ret == IDOK) {
                options = opt.GetOptions();
                this->mPaperColor = options.PaperColor;
                this->mDefaultFgColor = options.FontColor;
                this->mDefaultBgColor = options.PaperColor;
                this->mFont = options.Font;
                this->mMaxLinesBufferSize = options.LinesBufferSize;
                int fontw = mFont.GetAveWidth(), fonth = mFont.GetLineHeight();
                int shrink_cnt = (int)mLinesBuffer.size() - options.LinesBufferSize;
                if (shrink_cnt > 0) {
                    mLinesBuffer.erase(mLinesBuffer.begin(), mLinesBuffer.begin() + shrink_cnt);
                }
                // will override the old parameters, such as line-height
                for (size_t k = 0; k < mLinesBuffer.size(); ++k) {
                    int h = mLinesBuffer[k].GetHeight();
                    mLinesBuffer[k].SetHeight((int)((double)h / mFontH * fonth));
                }
                for (size_t k = 0; k < mLines.size(); ++k) {
                    int h = mLines[k].GetHeight();
                    mLines[k].SetHeight((int)((double)h / mFontH * fonth));
                }
                mFontW = fontw, mFontH = fonth;
                // fix virtual screen.
                DoLayout();
                //
                UpdatePresentation();
            }
        });
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
                        PostCallback([=]() { RenderText(s); ProcessOverflowLines(); });
                    }
                    pending = true; // continue
                } else {
                    raw.push_back(buff[k]);
                }
            }
            if (!raw.empty()) {
                size_t ep; auto s = Utf8ToUtf32(raw, ep);
                raw = raw.substr(ep);
                PostCallback([=]() { RenderText(s); ProcessOverflowLines(); });
            }
            // callback is
            PostCallback([=]() { this->UpdatePresentation(); });
        } else std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
    }
}

int SerialConnVT::GetCharWidth(const VTChar& c)
{
    switch (c) {
    case '\t':
        return mFontW * atoi(c.Var("CellSize").c_str());
    case ' ': return mFontW;
    default: if (1) {
        // Save
        Color bg_color = mBgColor;
        Color fg_color = mFgColor;
        bool blink = mBlink;
        Font font = mFont;
        // we should use apply the attrs, so the GetWidth could return a correct value.
        c.ApplyAttrs();
        int cx = mFont.GetWidth((int)c);
        // Restore
        mBgColor = bg_color;
        mFgColor = fg_color;
        mFont = font;
        mBlink = blink;
        return cx <= 0 ? mFontW : cx;
    } break;
    }
    return 0;
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
//
VTLine* SerialConnVT::GetVTLine(int vy)
{
    VTLine* vline = nullptr;
    if (vy < (int)mLinesBuffer.size()) {
        vline = &mLinesBuffer[vy];
    } else {
        vline = &mLines[vy - (int)mLinesBuffer.size()];
    }
    return vline;
}
//
Point SerialConnVT::VirtualToLogic(const std::vector<VTLine>& lines, int vx, int vy)
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
Point SerialConnVT::VirtualToLogic(int vx, int vy)
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
int SerialConnVT::VirtualToLogic(const VTLine& vline, int vx)
{
    int lx = 0;
    for (int i = 0; i < vx; ++i) {
        lx += GetCharWidth(vline[i]);
    }
    return lx;
}
//
int SerialConnVT::GetLogicWidth(const VTLine& vline, int count)
{
    int x = 0;
    if (count < 0) count = (int)vline.size();
    for (int k = 0; k < count; ++k) {
        x += GetCharWidth(vline[k]);
    }
    return x;
}
//
// lx, ly - lines is virtual screen
Point SerialConnVT::LogicToVirtual(const std::vector<VTLine>& lines, int lx, int ly)
{
    // find vy
    int vy = -1, py = 0;
    for (int i = 0; i < (int)lines.size(); ++i) {
        py += lines[i].GetHeight();
        if (py > ly) {
            vy = i;
            break;
        }
    }
    if (vy < 0)
        return Point(-1, -1);
    int px = 0, vx = (int)lines[vy].size()-1;
    for (int i = 0; i < (int)lines[vy].size(); ++i) {
        px += GetCharWidth(lines[vy][i]);
        if (px > lx) {
            vx = i;
            break;
        }
    }
    return Point(vx, vy);
}
//
Point SerialConnVT::LogicToVirtual(int lx, int ly)
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
        vy = (int)(mLinesBuffer.size() + mLines.size()) - 1;
    if (vy < 0) return Point(-1, -1); // error
    const VTLine* vline = nullptr;
    if (vy < (int)mLinesBuffer.size()) {
        vline = &mLinesBuffer[vy];
    } else {
        vline = &mLines[vy - (int)mLinesBuffer.size()];
    }
    int px = 0, vx = (int)vline->size()-1;
    for (int k = 0; k < (int)vline->size(); ++k) {
        px += GetCharWidth(vline->at(k));
        if (px > lx) {
            vx = k;
            break;
        }
    }
    return Point(vx, vy);
}
//
int SerialConnVT::LogicToVirtual(const VTLine& vline, int lx)
{
    int px = 0, vx = (int)vline.size()-1;
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
// If the scrolling region was set, we should ignore those lines out of region
void SerialConnVT::ProcessOverflowLines()
{
    auto& span = mScrollingRegion;
    Size csz = GetConsoleSize();
    int bottom = span.Bottom;
    if (bottom < 0 || bottom >= csz.cy)
        bottom = csz.cy-1;
    ASSERT(span.Top < (int)mLines.size());
    // padding lines
    if (mVy == bottom+1) {
        PushToLinesBufferAndCheck(mLines[span.Top]);
        mLines.erase(mLines.begin() + span.Top);
        mLines.insert(mLines.begin() + bottom, VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
        mVy = bottom;
    } else if (mVy >= csz.cy) { // wrap lines, override the last line of virtual screen.
        mVy = csz.cy - 1;
    }
    // padding chars
    int cnt = csz.cx - (int)mLines[mVy].size();
    for (int i = 0; i < cnt; ++i) {
        mLines[mVy].push_back(mBlankChar);
    }
}
//
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
    return terminal::cursor_beam;
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
        // scroll over
        if (p.y < 0) {
            mSbV.PrevLine();
        } else if (p.y > GetSize().cy) {
            mSbV.NextLine();
        }
        if (p.x < 0) {
            mSbH.PrevLine();
        } else if (p.x > GetSize().cx) {
            mSbH.NextLine();
        }
        Point vpos = LogicToVirtual(mSbH.Get() + p.x, mSbV.Get() + p.y);
        mSelectionSpan.X1 = p.x - mSelectionSpan.x0 > 0 ? vpos.x + 1 : vpos.x - 1;
        mSelectionSpan.Y1 = vpos.y;
        //
        mSelectionSpan.x1 = p.x;
        mSelectionSpan.y1 = p.y;
        //
        Refresh();
    }
}

void SerialConnVT::LeftDown(Point p, dword)
{
    mPressed = true;
    Point vpos = this->LogicToVirtual(mSbH.Get() + p.x, mSbV.Get() + p.y);
    mSelectionSpan.X0 = vpos.x;
    mSelectionSpan.X1 = vpos.x;
    mSelectionSpan.Y0 = vpos.y;
    mSelectionSpan.Y1 = vpos.y;
    mSelectionSpan.x0 = mSelectionSpan.x1 = p.x;
    mSelectionSpan.y0 = mSelectionSpan.y1 = p.y;
    // capture, event the cursor move out of the client region.
    SetFocus();
    SetCapture();
}

void SerialConnVT::LeftDouble(Point p, dword)
{
    // Select the word under the caret
    Point vpos = this->LogicToVirtual(mSbH.Get() + p.x, mSbV.Get() + p.y);
    VTLine* vline = nullptr;
    if (vpos.y < 0 || vpos.x < 0) return;
    if (vpos.y < (int)mLinesBuffer.size()) {
        vline = &mLinesBuffer[vpos.y];
    } else if (vpos.y - (int)mLinesBuffer.size() < (int)mLines.size()) {
        vline = &mLines[vpos.y - (int)mLinesBuffer.size()];
    }
    if (vline && vpos.x < (int)vline->size()) {
        // well, let's go
        mSelectionSpan.Y0 = mSelectionSpan.Y1 = vpos.y;
        // forward
        int fx = vpos.x;
        while (fx >= 0 && vline->at(fx) != ' ') {
            mSelectionSpan.X0 = fx--;
        }
        // backward
        int bx = vpos.x;
        while (bx < (int)vline->size() && vline->at(bx) != ' ') {
            mSelectionSpan.X1 = 1 + bx++;
        }
        //
        Refresh();
    }
}

void SerialConnVT::LeftUp(Point p, dword)
{
    (void)p;
    if (mPressed) {
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
        default:break;
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
        default:break;
        }
    } else if (flags & K_CTRL) { // only CTRL+
        switch (key) {
        case K_HOME: mSbV.Set(0); break;
        case K_END: mSbV.End(); break;
        default:break;
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
void SerialConnVT::ExtendVirtualScreen(int cx, int cy)
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
                vline.push_back(mBlankChar);
            }
            mLines.insert(mLines.begin(), vline);
            ln++;
        }
        mVy += ln;
    }
    // 1. push blanks
    for (int i = ln; i < cy && i < extcn; ++i) {
        mLines.push_back(VTLine(cx, mBlankChar).SetHeight(mFontH));
    }
}

void SerialConnVT::ShrinkVirtualScreen(int cx, int cy)
{
    // shrink virtual screen
    // Shrink maybe destroy the scrolling region. We'll keep the scrolling region
    // as we can, but two conditions will destroy them, if that happened, we use
    // entire screen as scrolling region.
    int top = mScrollingRegion.Top;
    int bot = mScrollingRegion.Bottom;
    if (bot < 0) {
        bot = (int)mLines.size()-1;
    }
    // 0. remove blanks firstly
    int blankcnt = CalculateNumberOfBlankLinesFromEnd(mLines);
    int shkcn = (int)mLines.size() - cy;
    int ln = 0;
    for (int i = 0; i < shkcn && i < blankcnt; ++i) {
        mLines.pop_back();
        ln++;
        top--;
    }
    // 1. remove from head
    for (int i = ln; i < shkcn; ++i) {
        PushToLinesBufferAndCheck(*mLines.begin());
        mLines.erase(mLines.begin());
        if (mVy > 0) {
            mVy--;
        }
    }
    // set scrolling region to default.
    if (top < 0 || bot >= mLines.size()) {
        mScrollingRegion.Top = 0;
        mScrollingRegion.Bottom = -1;
    }
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
	    ExtendVirtualScreen(csz.cx, csz.cy);
	    //--------------------------------------------------------------------------------------
	} else {
	    ShrinkVirtualScreen(csz.cx, csz.cy);
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
        // strip tail blanks of lines or \v
        size_t n = 0;
        while (n < lines[k].size()) {
            uint32_t c = lines[k][lines[k].size()-1-n];
            if (c == ' ' || c == '\n' || c == '\v')
                n++;
            else break;
        }
        out += lines[k].substr(0, lines[k].size() - n);
        if (k + 1 != lines.size()) {
            out += '\n';
        }
    }
    return out;
}
//
bool SerialConnVT::IsCharInSelectionSpan(int vx, int vy) const
{
    auto span = mSelectionSpan; bool lr = true;
    if (span.Y0 > span.Y1) { // top left
        std::swap(span.Y0, span.Y1);
        std::swap(span.X0, span.X1);
    } else if (span.Y0 == span.Y1) {
        if (span.X0 > span.X1) {
            std::swap(span.X0, span.X1);
            lr = false;
        }
    }
    // calculate absolute position
    if (span.X1 - span.X0 <= 0 &&
        span.Y1 - span.Y0 <= 0) return false;
    int abs_x = vx, abs_y = vy;
    if (abs_y == span.Y0) { // head line
        if (span.Y1-span.Y0 == 0) {
            if (lr) {
                if (abs_x >= span.X0 && abs_x < span.X1) return true;
            } else {
                if (abs_x > span.X0 && abs_x <= span.X1) return true;
            }
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
    draw.DrawRect(mPx, mPy, 2, mFontH, Color(0, 255, 0));
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
    int x = lxoff, y = lyoff, i = 0;
    bool tail_selected = IsCharInSelectionSpan((int)vline.size()-1, vy);
    bool line_selected = IsCharInSelectionSpan(0, vy) && tail_selected; // line was selected.
    for (i = vx; i < (int)vline.size() && x < usz.cx; ++i) {
        vline[i].ApplyAttrs();
        bool is_selected = line_selected ? true : IsCharInSelectionSpan(i, vy);
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
        if (is_selected) { // restore color
            std::swap(mBgColor, mFgColor);
        }
    }
    if (IsCharInSelectionSpan(0, vy+1) && tail_selected) { // we need to pad the blank
        if (usz.cx - x > 0) {
            // Use black to tell the user it's the end of the normal text.
            draw.DrawRect(x, y, usz.cx-x, vline.GetHeight(), Color(0, 0, 0));
        }
    }
    // apply attrs of invisible chars
    for (int k = i; k < (int)vline.size(); ++k) {
        vline[k].ApplyAttrs();
    }
}

void SerialConnVT::UpdatePresentationPos()
{
    Point ppos = VirtualToLogic(mVx, (int)mLinesBuffer.size() + mVy);
    mPx = ppos.x - mSbH.Get();
    mPy = ppos.y - mSbV.Get();
}

void SerialConnVT::UpdateVScrollbar()
{
    mSbV.SetTotal((int)(GetVTLinesHeight(mLines) + GetVTLinesHeight(mLinesBuffer)));
    if (mScrollToEnd) {
        mSbV.End();
    }
}

void SerialConnVT::UpdateHScrollbar()
{
    Point vpos = LogicToVirtual(0, mSbV.Get());
    if (vpos.x < 0 || vpos.y < 0) return;
    // let's begin
    Size usz = GetSize(); int vy = vpos.y;
    int longest_linesz = 0, lyoff = 0;
    for (int i = vpos.y; i < (int)mLinesBuffer.size() && lyoff < usz.cy; ++i) {
        const VTLine& vline = mLinesBuffer[i];
        lyoff += vline.GetHeight();
        int linesz = this->GetLogicWidth(vline, (int)vline.size() - this->CalculateNumberOfBlankCharsFromEnd(vline));
        if (linesz > longest_linesz) {
            longest_linesz = linesz;
        }
    }
    for (int i = std::max(0, vpos.y - (int)mLinesBuffer.size()); i < (int)mLines.size() && lyoff < usz.cy; ++i) {
        const VTLine& vline = mLines[i];
        lyoff += vline.GetHeight();
        int linesz = this->GetLogicWidth(vline, (int)vline.size() - this->CalculateNumberOfBlankCharsFromEnd(vline));
        if (linesz > longest_linesz) {
            longest_linesz = linesz;
        }
    }
    mSbH.SetTotal(longest_linesz);
}

void SerialConnVT::UpdatePresentation()
{
    UpdateVScrollbar();
    UpdateHScrollbar();
    UpdatePresentationPos();
    //
    Refresh();
}

void SerialConnVT::DrawVT(Draw& draw)
{
    // calculate offset
    int sbh = mSbH.Get(), sbv = mSbV.Get();
    Point vpos = LogicToVirtual(sbh, sbv);
    if (vpos.x < 0 || vpos.y < 0) return;
    Point lpos = VirtualToLogic(vpos.x, vpos.y);
    int lyoff = lpos.y - mSbV.Get();
    // use bot
    int bot = mScrollingRegion.Bottom;
    if (bot < 0) {
        bot = (int)mLines.size() - 1;
    }
    //--------------------------------------------------------------
    // draw lines, and calculate the presentation information
    Size usz = GetSize(); int vy = vpos.y;
    for (int i = vpos.y; i < (int)mLinesBuffer.size() && lyoff < usz.cy; ++i) {
        const VTLine& vline = mLinesBuffer[i];
        int vx = this->LogicToVirtual(vline, sbh);
        int lx = this->VirtualToLogic(vline, vx);
        int lxoff = lx - sbh;
        DrawVTLine(draw, vline, vx, vy++, lxoff, lyoff);
        lyoff += vline.GetHeight();
    }
    for (int i = std::max(0, vpos.y - (int)mLinesBuffer.size()); i < (int)mLines.size() && lyoff < usz.cy; ++i) {
        const VTLine& vline = mLines[i];
        int vx = this->LogicToVirtual(vline, sbh);
        int lx = this->VirtualToLogic(vline, vx);
        int lxoff = lx - sbh;
        DrawVTLine(draw, vline, vx, vy++, lxoff, lyoff);
        lyoff += vline.GetHeight();
    }
}

void SerialConnVT::Render(Draw& draw)
{
    // draw background
    draw.DrawRect(GetRect(), mPaperColor);
    //
    DrawVT(draw);
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
