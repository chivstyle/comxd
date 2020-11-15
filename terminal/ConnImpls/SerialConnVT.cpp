//
// (c) 2020 chiv
//
#include "terminal_rc.h"
#include "SerialConnVT.h"
#include "ConnFactory.h"
#include "ControlSeq.h"
#include "TextCodecsDialog.h"
#include "VTOptionsDialog.h"
#include <algorithm>

// REGISTER_CONN_INSTANCE("Original VT", SerialConnVT);
// register
using namespace Upp;
//----------------------------------------------------------------------------------------------
SerialConnVT::SerialConnVT(std::shared_ptr<SerialIo> serial)
    : SerialConn(serial)
    , mPx(0)
    , mPy(0)
    , mVx(0)
    , mVy(0)
    , mRxShouldStop(false)
    , mBlinkSignal(true)
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
    mBlankChar = ' ';
    // enable scroll bar
    mSbH.SetLine(mFontW);
    mSbH.Set(0);
    mSbV.SetTotal(0);
    AddFrame(mSbH);
    mSbH.WhenScroll = [=]() {
        UpdatePresentationPos();
        Refresh();
    };
    // vertical
    mSbV.SetLine(mFontH);
    mSbV.Set(0);
    mSbV.SetTotal(0);
    AddFrame(mSbV);
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
    mBlinkTimer.Set(-500, [&]() {
        mBlinkSignal = !mBlinkSignal;
        this->Refresh();
    });
    //
    InstallUserActions();
    // finally start the rx-thread.
    mRxThr = std::thread([=]() { RxProc(); });
}

SerialConnVT::~SerialConnVT()
{
    mRxShouldStop = true;
    if (mRxThr.joinable()) {
        mRxThr.join();
    }
}
//
void SerialConnVT::InstallUserActions()
{
    mUsrActions.emplace_back(terminal::text_codec(),
        t_("Text Codec"), t_("Select a text codec"), [=]() {
            TextCodecsDialog d(GetCodec()->GetName().c_str());
            int ret = d.Run();
            if (ret == IDOK) {
                this->SetCodec(d.GetCodecName());
                Refresh();
            }
        });
    mUsrActions.emplace_back(terminal::clear_buffer(),
        t_("Clear Buffer"), t_("Clear the line buffers"), [=]() { Clear(); });
    mUsrActions.emplace_back(terminal::vt_options(),
        t_("VT options"), t_("Virtual terminal options"), [=]() {
            VTOptionsDialog::Options options;
            options.Font = mFont;
            options.LinesBufferSize = (int)this->mMaxLinesBufferSize;
            options.PaperColor = mColorTbl.GetColor(VTColorTable::kColorId_Paper);
            options.FontColor = mColorTbl.GetColor(VTColorTable::kColorId_Texts);
            VTOptionsDialog opt;
            opt.SetOptions(options);
            int ret = opt.Run();
            if (ret == IDOK) {
                options = opt.GetOptions();
                this->mFont = options.Font;
                this->mMaxLinesBufferSize = options.LinesBufferSize;
                this->mColorTbl.SetColor(VTColorTable::kColorId_Texts, options.FontColor);
                this->mColorTbl.SetColor(VTColorTable::kColorId_Paper, options.PaperColor);
                //
                bool vscr_modified = false; // lines buffer or virtual screen was modified ?
                int fontw = mFont.GetAveWidth(), fonth = mFont.GetLineHeight();
                if (fontw != mFontW || fonth != mFontH) {
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
                    vscr_modified = true;
                }
                if (vscr_modified) {
                    DoLayout();
                    UpdatePresentation();
                }
            }
        });
}
//
void SerialConnVT::SaveScr(ScreenData& sd)
{
    sd.Vx = mVx;
    sd.Vy = mVy;
    sd.Style = mStyle;
    sd.Font = mFont;
    sd.Lines = mLines;
    sd.LinesBuffer = mLinesBuffer;
    sd.SelSpan = mSelectionSpan;
}

void SerialConnVT::SwapScr(ScreenData& sd)
{
    std::swap(sd.Vx, mVx);
    std::swap(sd.Vy, mVy);
    std::swap(sd.Style, mStyle);
    std::swap(sd.Font, mFont);
    std::swap(sd.Lines, mLines);
    std::swap(sd.LinesBuffer, mLinesBuffer);
    std::swap(sd.SelSpan, mSelectionSpan);
    // layout again.
    mFontW = mFont.GetAveWidth();
    mFontH = mFont.GetLineHeight();
    DoLayout();
    //
    UpdatePresentation();
}

void SerialConnVT::LoadScr(const ScreenData& sd)
{
    mLinesBuffer = sd.LinesBuffer;
    mLines = sd.Lines;
    mVx = sd.Vx;
    mVy = sd.Vy;
    mStyle = sd.Style;
    mFont = sd.Font;
    mSelectionSpan = sd.SelSpan;
    // restore font w/h
    mFontW = mFont.GetAveWidth();
    mFontH = mFont.GetLineHeight();
    // reset layout
    DoLayout();
    UpdatePresentation();
}
//
void SerialConnVT::SetDefaultStyle()
{
    mStyle = VTStyle(); // default.
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
    mScrollToEnd = true;
    //
    UpdatePresentation();
}
//
void SerialConnVT::RenderSeqs()
{
    std::lock_guard<std::mutex> _(mLockSeqs);
    while (!mSeqs.empty()) {
        auto seq = mSeqs.front();
        switch (seq.type) {
        case Seq::CTRL_SEQ:
            ProcessControlSeq(seq.ctrl.first, seq.ctrl.second);
            if (seq.ctrl.second != SEQ_NONE) {
                ProcessOverflowLines();
            }
            break;
        case Seq::TEXT_SEQ:
            RenderText(seq.text);
            break;
        }
        mSeqs.pop();
    }
    UpdatePresentation();
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
            // read raw, not string, so, we could read NUL from the device.
            std::vector<byte> buff = GetSerial()->ReadRaw(sz);
            for (size_t k = 0; k < sz; ++k) {
                if (pending) {
                    pattern.push_back((char)buff[k]);
                    int ret = IsControlSeq(pattern);
                    if (ret != SEQ_PENDING) { // bingo
                        // found it.
                        AddSeq(pattern, ret);
                        //
                        pending = false;
                        pattern = "";
                    }
                } else if (IsSeqPrefix(buff[k])) {
                    // come across a 0x1b, render the raw
                    if (!raw.empty()) {
                        size_t ep;
                        auto s = this->TranscodeToUTF32(raw, ep);
                        raw = raw.substr(ep);
                        if (!s.empty()) {
                            AddSeq(std::move(s));
                        }
                    }
                    pending = true; // continue
                } else {
                    raw.push_back(buff[k]);
                }
            }
            if (!raw.empty()) {
                size_t ep;
                auto s = this->TranscodeToUTF32(raw, ep);
                raw = raw.substr(ep);
                if (!s.empty()) {
                    AddSeq(std::move(s));
                }
            }
            // callback is
            PostCallback([=]() { RenderSeqs(); });
        } else {
            std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
        }
    }
}

int SerialConnVT::GetCharWidth(const VTChar& c, int vx)
{
    int cx = mFontW;
    switch (c.Code()) {
    case '\t':
        cx = cx * (8 - (vx % 8));
        break;
    case '\v':
    case ' ':
        break;
    default: if (1) {
            Font font = mFont; bool blink, visible;
            c.UseFontStyle(font, blink, visible);
            cx = font.GetWidth(c.Code());
        } break;
    }
    return cx;
}

bool SerialConnVT::IsSeqPrefix(unsigned char c)
{
    return c == 0x1b;
}

int SerialConnVT::IsControlSeq(const std::string& seq)
{
    (void)seq;
    return SEQ_NONE;
}

Size SerialConnVT::GetConsoleSize() const
{
    Size usz = GetSize();
    if (usz.cx <= 0 || usz.cy <= 0) {
        int cx = mLines.empty() ? 0 : (int)mLines[0].size();
        int cy = (int)mLines.size();
        return Size(cx, cy);
    }
    return Size(usz.cx / mFontW, usz.cy / mFontH);
}
//
VTLine* SerialConnVT::GetVTLine(int vy)
{
    VTLine* vline = nullptr;
    if (vy >= 0 && vy < (int)(mLinesBuffer.size() + mLines.size())) {
	    if (vy < (int)mLinesBuffer.size()) {
	        vline = &mLinesBuffer[vy];
	    } else {
	        vline = &mLines[vy - (int)mLinesBuffer.size()];
	    }
    }
    return vline;
}
const VTLine* SerialConnVT::GetVTLine(int vy) const
{
    const VTLine* vline = nullptr;
    if (vy >= 0 && vy < (int)(mLinesBuffer.size() + mLines.size())) {
	    if (vy < (int)mLinesBuffer.size()) {
	        vline = &mLinesBuffer[vy];
	    } else {
	        vline = &mLines[vy - (int)mLinesBuffer.size()];
	    }
    }
    return vline;
}
//
Point SerialConnVT::VirtualToLogic(const std::vector<VTLine>& lines, int vx, int vy)
{
    if (vy < 0 || vy >= (int)lines.size())
        return Size(-1, -1);
    int ly = 0;
    for (size_t k = 0; k < vy && k < lines.size(); ++k) {
        ly += lines[k].GetHeight();
    }
    int lx = 0;
    for (size_t k = 0; k < vx && k < lines[vy].size(); ++k) {
        lx += GetCharWidth(lines[vy][k], (int)k);
    }
    return Point(lx, ly);
}
//
Point SerialConnVT::VirtualToLogic(int vx, int vy)
{
    const VTLine* vline = this->GetVTLine(vy);
    if (!vline)
        return Point(-1, -1);
    // calculate ly
    int ly = 0;
    for (size_t k = 0; k < (int)mLinesBuffer.size() && k < vy; ++k) {
        ly += mLinesBuffer[k].GetHeight();
    }
    for (int k = (int)mLinesBuffer.size(); k < vy; ++k) {
        ly += mLines[k - (int)mLinesBuffer.size()].GetHeight();
    }
    // calculate lx
    int lx = 0;
    for (size_t k = 0; k < vx && k < vline->size(); ++k) {
        lx += GetCharWidth(vline->at(k), (int)k);
    }
    return Point(lx, ly);
}
//
int SerialConnVT::VirtualToLogic(const VTLine& vline, int vx)
{
    int lx = 0;
    for (size_t i = 0; i < vx && i < vline.size(); ++i) {
        lx += GetCharWidth(vline[i], (int)i);
    }
    return lx;
}
//
int SerialConnVT::GetLogicWidth(const VTLine& vline, int count)
{
    int x = 0;
    if (count < 0)
        count = (int)vline.size();
    for (int k = 0; k < count; ++k) {
        x += GetCharWidth(vline[k], (int)k);
    }
    return x;
}
//
// lx, ly - lines is virtual screen
Point SerialConnVT::LogicToVirtual(const std::vector<VTLine>& lines, int lx, int ly, int& px, int& next_px,
    int& py, int& next_py)
{
    if (lx < 0)
        lx = 0;
    if (ly < 0)
        ly = 0; // 0~
    int vy = (int)lines.size() - 1;
    py = 0;
    next_py = 0;
    for (int i = 0; i < (int)lines.size(); ++i) {
        int line_sz = lines[i].GetHeight();
        next_py += line_sz;
        //
        if (ly >= py && ly < next_py) {
            vy = i;
            break;
        }
        py += line_sz;
    }
    int vx = (int)lines[vy].size() - 1;
    px = 0;
    next_px = 0;
    for (int i = 0; i < (int)lines[vy].size(); ++i) {
        int vchar_sz = GetCharWidth(lines[vy][i], i);
        next_px += vchar_sz;
        // [px, px+vchar_sz]
        if (lx >= px && lx < next_px) {
            vx = i;
            break;
        }
        //
        px += vchar_sz;
    }
    return Point(vx, vy);
}
//
Point SerialConnVT::LogicToVirtual(int lx, int ly, int& px, int& next_px,
    int& py, int& next_py)
{
    if (lx < 0)
        lx = 0;
    if (ly < 0)
        ly = 0; // 0~
    int vy = -1;
    py = 0;
    next_py = 0;
    for (int i = 0; i < (int)mLinesBuffer.size(); ++i) {
        int line_sz = mLinesBuffer[i].GetHeight();
        next_py += line_sz;
        if (ly >= py && ly < next_py) {
            vy = i;
            break;
        }
        py += line_sz;
    }
    if (vy < 0) {
        vy = (int)(mLinesBuffer.size()+mLines.size())-1;
        for (int i = 0; i < (int)mLines.size(); ++i) {
            int line_sz = mLines[i].GetHeight();
            next_py += line_sz;
            if (ly >= py && ly < next_py) {
                vy = (int)mLinesBuffer.size() + i;
                break;
            }
            py += line_sz;
        }
    }
    if (vy < 0)
        return Point(-1, -1); // Error, what's wrong? Because lines and linesbuffer all all empty.
    //
    const VTLine* vline = this->GetVTLine(vy);
    int vx = (int)vline->size() - 1;
    px = 0;
    next_px = 0;
    for (int k = 0; k < (int)vline->size(); ++k) {
        int vchar_sz = GetCharWidth(vline->at(k), (int)k);
        next_px += vchar_sz;
        // [px, px+vchar_sz]
        if (lx >= px && lx < next_px) {
            vx = k;
            break;
        }
        // next
        px += vchar_sz;
    }
    return Point(vx, vy);
}
//
int SerialConnVT::LogicToVirtual(const VTLine& vline, int lx, int& px, int& next_px)
{
    if (lx < 0)
        lx = 0; // 0~
    int vx = (int)vline.size() - 1;
    px = 0;
    next_px = 0;
    for (int i = 0; i < (int)vline.size(); ++i) {
        int vchar_sz = GetCharWidth(vline[i], i);
        next_px += vchar_sz;
        // [px, px+vchar_sz]
        if (lx >= px && lx < next_px) {
            vx = i;
            break;
        }
        //
        px += vchar_sz;
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
    case '\v': // VT
        mLines[mVy][mVx] = '\v';
        mVy++;
        break;
    case '\b': // BS
        if (mVx > 0) {
            mVx--;
        }
        break;
    case '\r': // CR
        mVx = 0;
        break;
    case '\n': // LF
        mVy += 1;
        break;
    case '\t':
        mLines[mVy][mVx] = '\t';
        mVx++;
        break;
    }
    return true;
}
//
bool SerialConnVT::ProcessControlSeq(const std::string& seq, int seq_type)
{
    if (seq_type == SEQ_NONE) { // can't recognize the seq, display it
        LOGF("Unrecognized:%s\n", seq.c_str());
        size_t ep;
        RenderText(this->TranscodeToUTF32(seq, ep));
        return true;
    }
    return false;
}
//
void SerialConnVT::CheckAndFix(ScrollingRegion& span)
{
	if (span.Top < 0) span.Top = 0;
	if (span.Top >= span.Bottom) {
		span.Top = 0;
		span.Bottom = -1;
	} else {
		if (span.Bottom >= (int)mLines.size()) {
			span.Bottom = -1;
		}
	}
}
// If the scrolling region was set, we should ignore those lines out of region
void SerialConnVT::ProcessOverflowLines()
{
    auto& span = mScrollingRegion;
    Size csz = GetConsoleSize();
    int bottom = span.Bottom;
    if (bottom < 0 || bottom >= csz.cy)
        bottom = csz.cy - 1;
    ASSERT(span.Top < bottom);
    // scrolling region.
    int dy = mVy - bottom;
    if (dy > 0) {
        mLines.insert(mLines.begin() + bottom + 1, dy, VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
        if (span.Top == 0) { // If span.Top == 0, buffer the lines out of scrolling region.
	        for (int i = span.Top; i < span.Top + dy; ++i) {
	            PushToLinesBufferAndCheck(mLines[i]);
	        }
        }
        mLines.erase(mLines.begin() + span.Top, mLines.begin() + span.Top + dy);
        // fixed
        mVy = bottom;
    }
    int dx = mVx - (int)mLines[mVy].size() + 1;
    if (dx > 0) {
        mLines[mVy].insert(mLines[mVy].end(), dx, mBlankChar);
    }
}
//
void SerialConnVT::RenderText(const std::vector<uint32_t>& s)
{
    Size csz = GetConsoleSize();
    for (size_t k = 0; k < s.size(); ++k) {
        VTChar chr = s[k];
        if (chr.Code() < 0x20 || chr.Code() == 0x7f) {
            if (ProcessAsciiControlChar(chr.Code())) {
                ProcessOverflowLines();
            }
        } else {
            chr.SetStyle(mStyle);
            // Unfortunately, UPP does not support complete UNICODE, support UCS-16 instead. So
            // we should ignore those out of range
            if (chr.Code() > 0xffff)
                chr.SetCode('?'); // replace chr with ?
            VTLine& vline = mLines[mVy];
            if (mVx < vline.size()) {
                vline[mVx] = chr;
            } else {
                // padding blanks if needed.
                for (int n = (int)vline.size() - 1; n < mVx - 1; ++n) {
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
    return terminal::cursor_beam();
}

void SerialConnVT::MouseWheel(Point, int zdelta, dword)
{
    mSbV.Wheel(zdelta);
}

int SerialConnVT::GetLineHeight(int vy) const
{
    const VTLine* vline = this->GetVTLine(vy);
    if (vline)
        return vline->GetHeight();
    else
        return 0;
}

void SerialConnVT::MouseMove(Point p, dword)
{
    if (mPressed) {
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
        int lx = mSbH.Get() + p.x, ly = mSbV.Get() + p.y;
        int px, next_px, py, next_py;
        Point vpos = LogicToVirtual(lx, ly, px, next_px, py, next_py);
        mSelectionSpan.X1 = (lx - px) > (next_px - px) / 2 ? vpos.x + 1 : vpos.x;
        mSelectionSpan.Y1 = vpos.y;
        mSelectionSpan.Valid = mSelectionSpan.X0 != mSelectionSpan.X1 || mSelectionSpan.Y0 != mSelectionSpan.Y1;
        //
        Refresh();
    }
}

void SerialConnVT::LeftDown(Point p, dword)
{
    mPressed = true;
    int lx = mSbH.Get() + p.x, ly = mSbV.Get() + p.y;
    int px, next_px, py, next_py;
    Point vpos = LogicToVirtual(lx, ly, px, next_px, py, next_py);
    mSelectionSpan.X0 = (lx - px) > (next_px - px) / 2 ? vpos.x + 1 : vpos.x;
    mSelectionSpan.Y0 = vpos.y;
    mSelectionSpan.X1 = mSelectionSpan.X0;
    mSelectionSpan.Y1 = mSelectionSpan.Y0;
    mSelectionSpan.Valid = false;
    // show the caret
    VTLine* vline = this->GetVTLine(vpos.y);
    if (vline) {
        Point lpos = VirtualToLogic(mSelectionSpan.X0, mSelectionSpan.Y0);
        this->SetCaret(lpos.x - mSbH.Get(), lpos.y - mSbV.Get(), 1, vline->GetHeight());
    }
    // capture, event the cursor move out of the client region.
    SetFocus();
    SetCapture();
}

void SerialConnVT::LeftDouble(Point p, dword)
{
    // Select the word under the caret
    int lx = mSbH.Get() + p.x, ly = mSbV.Get() + p.y;
    int px, next_px, py, next_py;
    Point vpos = LogicToVirtual(lx, ly, px, next_px, py, next_py);
    VTLine* vline = this->GetVTLine(vpos.y);
    if (vline && vpos.x < (int)vline->size()) {
        // well, let's go
        mSelectionSpan.Y0 = mSelectionSpan.Y1 = vpos.y;
        // forward
        int fx = vpos.x;
        mSelectionSpan.X0 = fx;
        while (fx >= 0 && vline->at(fx).Code() != ' ') {
            mSelectionSpan.X0 = fx--;
        }
        // backward
        int bx = vpos.x + 1; // selection span is [vpos.x, vpos.x + N+1)
        if (fx != vpos.x) { // If you have clicked blank, we do not extend the selection span
            while (bx < (int)vline->size() && vline->at(bx).Code() != ' ') {
                bx++;
            }
        }
        mSelectionSpan.X1 = bx;
        // Create a condition
        mSelectionSpan.Valid = mSelectionSpan.X1 != mSelectionSpan.X0;
        //
        Refresh();
    }
}

void SerialConnVT::LeftTriple(Point p, dword)
{
    int lx = mSbH.Get() + p.x, ly = mSbV.Get() + p.y;
    int px, next_px, py, next_py;
    Point vpos = LogicToVirtual(lx, ly, px, next_px, py, next_py);
    VTLine* vline = this->GetVTLine(vpos.y);
    if (vline) {
        mSelectionSpan.Y0 = mSelectionSpan.Y1 = vpos.y;
        mSelectionSpan.X0 = 0;
        mSelectionSpan.X1 = (int)vline->size();
        mSelectionSpan.Valid = true;
        Refresh();
    }
}

void SerialConnVT::LeftUp(Point p, dword)
{
    (void)p;
    if (mPressed) {
        this->KillCaret();
        mPressed = false;
        Refresh();
    }
    ReleaseCapture();
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
    if (!mLines.empty()) {
        mSelectionSpan.X0 = 0;
        mSelectionSpan.X1 = 0;
        mSelectionSpan.Y0 = 0;
        mSelectionSpan.Y1 = (int)mLines.size();
        mSelectionSpan.Valid = true;
        //
        Refresh();
    }
}

void SerialConnVT::RightUp(Point, dword)
{
    MenuBar::Execute([=](Bar& bar) {
        bool has_sel = mSelectionSpan.Valid;
        bar.Add(has_sel, t_("Copy"), [=] {
               // copy to clipboard
               Copy();
           })
            .Image(CtrlImg::copy())
            .Key(K_CTRL | K_SHIFT | K_C);
        String text = ReadClipboardUnicodeText().ToString();
        bar.Add(text.GetCount() > 0, t_("Paste"), [=] {
               Paste();
           })
            .Image(CtrlImg::paste())
            .Key(K_CTRL | K_SHIFT | K_V);
        bar.Add(true, t_("Select all"), [=]() {
               SelectAll();
           })
            .Key(K_CTRL | K_SHIFT | K_A);
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
        default:
            break;
        }
    } else if ((flags & (K_CTRL | K_SHIFT)) == (K_CTRL | K_SHIFT)) { // CTRL+SHIFT+
        switch (key) {
        case K_C:
            Copy();
            return true;
        case K_A:
            SelectAll();
            return true;
        case K_V:
            Paste();
            return true;
        default:
            break;
        }
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
        default:
            break;
        }
    } else if ((flags & K_CTRL) == K_CTRL) {
        if (key >= K_A && key <= K_Z) {
            d.push_back(1 + (uint8_t)(key - K_A));
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
        // scroll to end when the user input valid chars.
        mSbV.End();
        //
        GetSerial()->Write(d);
        return true;
    }
    return false;
}

bool SerialConnVT::ProcessKeyUp(dword key, dword flags)
{
    if ((flags & (K_CTRL | K_SHIFT)) == (K_CTRL | K_SHIFT)) { // CTRL+SHIFT+
        //
    } else if (flags & K_CTRL) { // only CTRL+
        switch (key) {
        case K_HOME:
            mSbV.Set(0);
            break;
        case K_END:
            mSbV.End();
            break;
        default:
            break;
        }
    }
    return false;
}

bool SerialConnVT::ProcessChar(dword cc)
{
    GetSerial()->Write(GetCodec()->TranscodeFromUTF32(cc));
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
            if (vline[k].Code() != ' ') {
                blank = false;
                break;
            }
        }
        if (blank) {
            cn++;
        } else
            break;
    }
    return cn;
}

int SerialConnVT::CalculateNumberOfBlankCharsFromEnd(const VTLine& vline) const
{
    int cn = 0;
    size_t sz = vline.size();
    while (sz--) {
        if (vline[sz].Code() == ' ')
            cn++;
        else
            break;
    }
    return cn;
}
//
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
            VTLine vline = *mLinesBuffer.rbegin();
            mLinesBuffer.pop_back();
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
        bot = (int)mLines.size() - 1;
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
}
//
void SerialConnVT::DoLayout()
{
    Size csz = GetConsoleSize();
    mSbV.SetPage(mFontH * csz.cy);
    mSbH.SetPage(mFontW * csz.cx);
    if (csz.cx <= 0 || csz.cy <= 0)
        return;
    // check and fix
    for (size_t k = 0; k < mLines.size(); ++k) {
        VTLine& vline = mLines[k];
        for (size_t i = vline.size(); (int)i < csz.cx; ++i) {
            vline.push_back(mBlankChar);
        }
        int overflow_cnt = (int)vline.size() - csz.cx;
        if (overflow_cnt > 0) {
            // erase those blanks out of range
            int blanks_cnt = this->CalculateNumberOfBlankCharsFromEnd(vline);
            int k = 0;
            while (k < overflow_cnt && k < blanks_cnt) {
                k++;
                vline.pop_back();
            }
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
    if (span.X0 == span.X1 && span.Y0 == span.Y1)
        return std::vector<std::string>();
    // merge buffer and virtual screen.
    int nlines = (int)mLines.size() - this->CalculateNumberOfBlankLinesFromEnd(mLines);
    std::vector<std::string> out(span.Y1 - span.Y0 + 1);
    size_t out_p = 0;
    // first line
    if (1) {
        const VTLine* vline = this->GetVTLine(span.Y0);
        std::string sel;
        int nchars = (int)vline->size() - this->CalculateNumberOfBlankCharsFromEnd(*vline);
        int vx = span.X0;
        for (size_t i = vx; i < nchars; ++i) {
            bool is_selected = IsCharInSelectionSpan((int)i, span.Y0);
            if (is_selected) {
                sel += UTF32ToUTF8_(vline->at(i).Code());
            }
        }
        out[out_p++] = sel;
    }
    // in span
    //------------------------------------------------------------------------------------------
    for (int k = span.Y0 + 1; k < span.Y1 && k < nlines; ++k) {
        const VTLine* vline = this->GetVTLine(k);
        std::string sel;
        int nchars = (int)vline->size() - this->CalculateNumberOfBlankCharsFromEnd(*vline);
        for (size_t i = 0; i < nchars; ++i) {
            sel += UTF32ToUTF8_(vline->at(i).Code());
        }
        out[out_p++] = sel;
    }
    // tail line, if existed
    if (span.Y1 > span.Y0 && span.Y1 < nlines) {
        const VTLine* vline = this->GetVTLine(span.Y1);
        std::string sel;
        int nchars = (int)vline->size() - this->CalculateNumberOfBlankCharsFromEnd(*vline);
        int vx = span.X1;
        for (size_t i = 0; i < nchars && i < vx; ++i) {
            bool is_selected = IsCharInSelectionSpan((int)i, span.Y1);
            if (is_selected) {
                sel += UTF32ToUTF8_(vline->at(i).Code());
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
            uint32_t c = lines[k][lines[k].size() - 1 - n];
            if (c == ' ' || c == '\n' || c == '\v')
                n++;
            else
                break;
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
    auto span = mSelectionSpan;
    if (!span.Valid)
        return false;
    if (span.Y0 > span.Y1) { // top left
        std::swap(span.Y0, span.Y1);
        std::swap(span.X0, span.X1);
    } else if (span.Y0 == span.Y1) {
        if (span.X0 > span.X1) {
            std::swap(span.X0, span.X1);
        }
    }
    if (vy == span.Y0) { // head line
        if (span.Y1 - span.Y0 == 0) {
            return vx >= span.X0 && vx < span.X1;
        } else {
            return vx >= span.X0;
        }
    } else if (vy == span.Y1) { // tail.
        return vx < span.X1;
    } else {
        if (vy > span.Y0 && vy < span.Y1) {
            return true;
        }
    }
    return false;
}

void SerialConnVT::DrawCursor(Draw& draw)
{
    Size usz = GetSize();
    if (mPx >= 0 && mPx < usz.cx && mPy >= 0 && mPy < usz.cy) {
        draw.DrawRect(mPx, mPy, mFontW, mFontH, Color(0, 255, 0));
        // a visible char.
        if (mVx < (int)mLines[mVy].size() && mVy < (int)mLines.size()
            && mLines[mVy][mVx].Code() != ' ' && mLines[mVy][mVx].Code() != '\t') {
            this->DrawVTChar(draw, mPx, mPy, mLines[mVy][mVx], mFont, \
                mColorTbl.GetColor(VTColorTable::kColorId_Paper));
        }
    }
}

int SerialConnVT::GetVTLinesHeight(const std::vector<VTLine>& lines) const
{
    int linesz = 0;
    for (size_t k = 0; k < lines.size(); ++k) {
        linesz += lines[k].GetHeight();
    }
    return linesz;
}

void SerialConnVT::UseStyle(const VTChar& c, Upp::Font& font, Upp::Color& fg_color, Upp::Color& bg_color,
    bool& blink, bool& visible)
{
	uint16_t fgcolor_id, bgcolor_id;
	c.UseStyle(font, fgcolor_id, bgcolor_id, blink, visible);
	fg_color = mColorTbl.GetColor(fgcolor_id);
	bg_color = mColorTbl.GetColor(bgcolor_id);
}

void SerialConnVT::DrawVTLine(Draw& draw, const VTLine& vline,
    int vx, int vy, /*! absolute position of data */
    int lxoff, int lyoff)
{
    Size usz = GetSize(), csz = GetConsoleSize();
    int x = lxoff, y = lyoff, i = 0;
    bool tail_selected = IsCharInSelectionSpan((int)vline.size() - 1, vy);
    bool line_selected = IsCharInSelectionSpan(0, vy) && tail_selected; // line was selected.
    int abc_cnt = (int)vline.size();
    // style
    const Color& paper_color = mColorTbl.GetColor(VTColorTable::kColorId_Paper);
    Color bg_color, fg_color; bool blink, visible;
    for (i = vx; i < abc_cnt && x < usz.cx; ++i) {
        UseStyle(vline[i], mFont, fg_color, bg_color, blink, visible);
        bool is_selected = line_selected ? true : IsCharInSelectionSpan(i, vy);
        if (is_selected) {
            std::swap(bg_color, fg_color);
        }
        int vchar_cx = GetCharWidth(vline[i], i);
        if (blink) {
            if (mBlinkSignal) {
                draw.DrawRect(x, y, vchar_cx, vline.GetHeight(), paper_color);
            } else {
                if (bg_color != paper_color) {
                    draw.DrawRect(x, y, vchar_cx, vline.GetHeight(), bg_color);
                }
                if (fg_color != bg_color) {
                    if (vline[i].Code() != ' ' && vline[i].Code() != '\t' && vline[i].Code() != '\v' && visible) {
                        DrawVTChar(draw, x, y, vline[i], mFont, fg_color);
                    }
                }
            }
        } else {
            if (bg_color != paper_color) {
                draw.DrawRect(x, y, vchar_cx, vline.GetHeight(), bg_color);
            }
            if (fg_color != bg_color) {
                if (vline[i].Code() != ' ' && vline[i].Code() != '\t' && vline[i].Code() != '\v' && visible) {
                    DrawVTChar(draw, x, y, vline[i], mFont, fg_color);
                }
            }
        }
        x += vchar_cx;
        if (is_selected) { // restore color
            std::swap(bg_color, fg_color);
        }
    }
    // should we padding line with blanks?
    if (tail_selected && vx < std::max(mSelectionSpan.Y0, mSelectionSpan.Y1)) {
        if (usz.cx - x > 0) {
            // Use black to tell the user it's the end of the normal text.
            draw.DrawRect(x, y, usz.cx - x, vline.GetHeight(), fg_color);
        }
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
    int lx = 0, ly = mSbV.Get();
    int px, next_px, py, next_py;
    Point vpos = LogicToVirtual(lx, ly, px, next_px, py, next_py);
    if (vpos.x < 0 || vpos.y < 0)
        return;
    int lyoff = py - ly;
    // let's begin
    Size usz = GetSize();
    int vy = vpos.y;
    int cursor_vy = mVy;
    int longest_linesz = 0;
    for (int i = vpos.y; i < (int)mLinesBuffer.size() && lyoff < usz.cy; ++i) {
        const VTLine& vline = mLinesBuffer[i];
        lyoff += vline.GetHeight();
        int nchars = cursor_vy != i ? (int)vline.size() - this->CalculateNumberOfBlankCharsFromEnd(vline) : -1;
        int linesz = this->GetLogicWidth(vline, nchars);
        if (linesz > longest_linesz) {
            longest_linesz = linesz;
        }
    }
    for (int i = std::max(0, vpos.y - (int)mLinesBuffer.size()); i < (int)mLines.size() && lyoff < usz.cy; ++i) {
        const VTLine& vline = mLines[i];
        lyoff += vline.GetHeight();
        int nchars = cursor_vy != i ? (int)vline.size() - this->CalculateNumberOfBlankCharsFromEnd(vline) : -1;
        int linesz = this->GetLogicWidth(vline, nchars);
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
    int lx = mSbH.Get(), ly = mSbV.Get();
    int px, next_px, py, next_py;
    Point vpos = LogicToVirtual(lx, ly, px, next_px, py, next_py);
    if (vpos.x < 0 || vpos.y < 0)
        return;
    int lyoff = py - ly;
    //--------------------------------------------------------------
    // draw lines, and calculate the presentation information
    Size usz = GetSize();
    int vy = vpos.y;
    for (int i = vpos.y; i < (int)mLinesBuffer.size() && lyoff < usz.cy; ++i) {
        const VTLine& vline = mLinesBuffer[i];
        int vx = this->LogicToVirtual(vline, lx, px, next_px);
        int lxoff = px - lx;
        DrawVTLine(draw, vline, vx, vy++, lxoff, lyoff);
        lyoff += vline.GetHeight();
    }
    for (int i = std::max(0, vpos.y - (int)mLinesBuffer.size()); i < (int)mLines.size() && lyoff < usz.cy; ++i) {
        const VTLine& vline = mLines[i];
        int vx = this->LogicToVirtual(vline, lx, px, next_px);
        int lxoff = px - lx;
        DrawVTLine(draw, vline, vx, vy++, lxoff, lyoff);
        lyoff += vline.GetHeight();
    }
}

void SerialConnVT::Render(Draw& draw)
{
    // draw background
    draw.DrawRect(GetRect(), mColorTbl.GetColor(VTColorTable::kColorId_Paper));
    //
    DrawVT(draw);
    //
    DrawCursor(draw);
}
// To UTF16
WString SerialConnVT::TranscodeToUTF16(const VTChar& cc) const
{
    // Because the UPP only support UCS-16, so we replace the unsupported chars with ?
    WString out;
    out << (int)cc.Code();
    return out;
}

std::vector<uint32_t> SerialConnVT::TranscodeToUTF32(const std::string& s, size_t& ep)
{
    return GetCodec()->TranscodeToUTF32((const unsigned char*)s.c_str(), s.length(), ep);
}

void SerialConnVT::DrawVTChar(Draw& draw, int x, int y, const VTChar& c,
    const Font& font, const Color& cr)
{
    // To UTF8
    WString text = TranscodeToUTF16(c);
    draw.DrawText(x, y, text, font, cr);
}

void SerialConnVT::Paint(Draw& draw)
{
    Render(draw);
}
