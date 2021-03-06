//
// (c) 2020 chiv
//
#include "terminal_rc.h"
#include "Charset.h"
#include "SerialConnVT.h"
#include "ControlSeq.h"
#include "TextCodecsDialog.h"
#include "VTOptionsDialog.h"
#include <algorithm>
//
#define ENABLE_H_SCROLLBAR           1
#define ENABLE_BLINK_TEXT            1
#define ENABLE_FIXED_LINE_HEIGHT     1
// register
using namespace Upp;
//----------------------------------------------------------------------------------------------
SerialConnVT::SerialConnVT(std::shared_ptr<SerialIo> io)
    : SerialConn(io)
    , mPx(0)
    , mPy(0)
    , mVx(0)
    , mVy(0)
    , mRxShouldStop(false)
    , mBlinkSignal(true)
    , mWrapLine(false)
    , mScrollToEnd(true)
    , mPressed(false)
    , mShowCursor(true)
    , mMaxLinesBufferSize(5000)
    , mSeqsFactory(new ControlSeqFactory())
    , mTabWidth(8)
    , mCharset(CS_DEFAULT)
    , mBackgroundColorId(VTColorTable::kColorId_Paper)
    , mForegroundColorId(VTColorTable::kColorId_Texts)
{
    // double buffer
    BackPaint();
    // default font
    mFont = VTOptionsDialog::DefaultFont();
    // handlers
    mFontW = std::max(mFont.GetWidth('M'), mFont.GetWidth('W'));
    mFontH = mFont.GetLineHeight();
    // default style
    mBlankChar.SetCode(' ');
#if ENABLE_H_SCROLLBAR
    // enable scroll bar
    mSbH.SetLine(mFontW);
    mSbH.Set(0);
    AddFrame(mSbH);
    mSbH.WhenScroll = [=]() {
        UpdatePresentationPos();
        Refresh();
    };
#endif
    // vertical
    mSbV.SetTotal(0);
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
#if ENABLE_BLINK_TEXT
    // blink timer
    mBlinkTimer.Set(-500, [&]() {
        mBlinkSignal = !mBlinkSignal;
        this->Refresh();
    });
#endif
    // initialize size
    VTLine vline = VTLine(80, mBlankChar).SetHeight(mFontH);
    mLines.insert(mLines.begin(), 30, vline);
    //
    InstallUserActions();
    //
    this->PostCallback([=]() {
        this->SetFocus();
        this->DoLayout();
    });
}

SerialConnVT::~SerialConnVT()
{
    Stop();
    delete mSeqsFactory;
}
//
bool SerialConnVT::Start()
{
    mRxShouldStop = false;
    mRxThr = std::thread([=]() { RxProc(); });
    //
    return true;
}
void SerialConnVT::Stop()
{
    mRxShouldStop = true;
    if (mRxThr.joinable()) {
        mRxThr.join();
    }
}
//
void SerialConnVT::ShowVTOptions()
{
    VTOptionsDialog::Options options;
    options.Font = mFont;
    options.LinesBufferSize = (int)this->mMaxLinesBufferSize;
    options.PaperColor = mColorTbl.GetColor(VTColorTable::kColorId_Paper);
    options.TextsColor = mColorTbl.GetColor(VTColorTable::kColorId_Texts);
    VTOptionsDialog opt;
    opt.SetOptions(options);
    int ret = opt.Run();
    if (ret == IDOK) {
        options = opt.GetOptions();
        this->mFont = options.Font;
        this->mMaxLinesBufferSize = options.LinesBufferSize;
        this->mColorTbl.SetColor(VTColorTable::kColorId_Texts, options.TextsColor);
        this->mColorTbl.SetColor(VTColorTable::kColorId_Paper, options.PaperColor);
        //
        bool vscr_modified = false; // lines buffer or virtual screen was modified ?
        int fontw = std::max(mFont.GetWidth('M'), mFont.GetWidth('W')), fonth = mFont.GetLineHeight();
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
}
//
void SerialConnVT::InstallUserActions()
{
    WhenUsrBar = [=](Bar& bar) {
        bar.Add(t_("Text Codec"), terminal::text_codec(), [=]() {
            TextCodecsDialog d(GetCodec()->GetName().c_str());
            int ret = d.Run();
            if (ret == IDOK) {
                this->SetCodec(d.GetCodecName());
                Refresh();
            }
        }).Help(t_("Select a text codec"));
        bar.Add(t_("Clear Buffer"), terminal::clear_buffer(), [=]() { Clear(); }).Help(t_("Clear the line buffers"));
        bar.Add(t_("VT Options"), terminal::vt_options(), [=]() {
            ShowVTOptions();
        }).Help(t_("Virtual terminal options"));
        //
        bar.Add(t_("Benchmark"), terminal::benchmark(), [=]() {
            RunParserBenchmark();
        }).Help(t_("Test the performance of the parser"));
    };
    
    WhenBar = [=](Bar& bar) {
        bool has_sel = mSelectionSpan.Valid;
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
    };
}
void SerialConnVT::SetWrapLine(bool b)
{
	mWrapLine = b;
}
void SerialConnVT::SetShowCursor(bool b)
{
	mShowCursor = b;
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
int SerialConnVT::IsControlSeq(const std::string_view& seq, size_t& p_begin, size_t& p_sz, size_t& s_end)
{
    return mSeqsFactory->IsControlSeq(seq, p_begin, p_sz, s_end);
}
//
bool SerialConnVT::ProcessControlSeq(int seq_type, const std::string_view& p)
{
    auto it = mFunctions.find(seq_type);
    if (it != mFunctions.end()) {
        it->second(p);
        // processed
        return true;
    }
    return false;
}
//
void SerialConnVT::RenderSeqs(const std::deque<Seq>& seqs)
{
    for (auto it = seqs.begin(); it != seqs.end(); ++it) {
        auto seq = *it;
        int flags = 0;
        //--------------------------------------------------------------------------------------
        int px = mPx, py = mPy, vx = mVx, vy = mVy;
        switch (seq.Type) {
        case Seq::CTRL_SEQ:
            ProcessControlSeq(seq.Ctrl.first, seq.Ctrl.second); // will change vx,vy,px,py
        break;
        case Seq::TEXT_SEQ: if (1) {
            int cx = (int)mLines[mVy].size();
            RenderText(seq.Text); // will change vx
            if (mVx > cx) flags |= 0x2; // the line was extended
        } break;
        }
        //--------------------------------------------------------------------------------------
        if (ProcessOverflowLines(seq)) { flags |= 0x1; }
        if (ProcessOverflowChars(seq)) { flags |= 0x2; } // fix vx/vy
#if 0
        if (flags & 0x1) { UpdateVScrollbar(); }
        if (flags & 0x2) { UpdateHScrollbar(); }
#endif
        // Update vx/vy
        if (mPx != px && mPy != py) {
            UpdateDataPos(); vx = mVx; vy = mVy; flags |= 0x4;
        } else if (mPx != px) {
            UpdateDataPos(0x1); vx = mVx; flags |= 0x4;
        } else if (mPy != py) {
            UpdateDataPos(0x2); vy = mVy; flags |= 0x4;
        }
        // update px/py
        if (mVx != vx && mVy != vy) {
            UpdatePresentationPos(); flags |= 0x4;
        } else if (mVx != vx) {
            UpdatePresentationPos(0x1); flags |= 0x4;
        } else if (mVy != vy) {
            UpdatePresentationPos(0x2); flags |= 0x4;
        }
    }
}
//
void SerialConnVT::RenderSeqs()
{
    mLockSeqs.lock();
    {
        RenderSeqs(mSeqs);
        mSeqs.clear();
    }
    mLockSeqs.unlock();
    //
    UpdateVScrollbar();
    UpdateHScrollbar();
    //
    Refresh();
}
//
bool SerialConnVT::IsControlSeqPrefix(uint8_t c)
{
	return c <= 0x1f || c == 0x7f;
}
//
void SerialConnVT::RefineTheInput(std::string& raw)
{
	(void)raw;
}
//
void SerialConnVT::Put(const std::string& s)
{
	GetIo()->Write(s);
}
//
size_t SerialConnVT::ParseSeqs(const std::string_view& raw, std::deque<struct Seq>& seqs)
{
    std::string texts;
    size_t rawp = 0;
    size_t seqs_cnt = 0;
    while (rawp < raw.length()) {
        if (IsControlSeqPrefix((uint8_t)raw[rawp])) { // is prefix of some control sequence
            if (!texts.empty()) {
                size_t ep; // end position
                auto s = this->TranscodeToUTF32(texts, ep);
                if (!s.empty()) {
                    seqs.emplace_back(std::move(s));
                    seqs_cnt++;
                }
                texts.clear();
            }
            size_t p_begin, p_sz, s_end;
            int type = IsControlSeq(raw.substr(rawp), p_begin, p_sz, s_end);
            if (type == SEQ_PENDING) {
                // should we go on ?
                if (*raw.rbegin() == '\r') { // CR will break the pending sequence
                    type = SEQ_CORRUPTED;
                } else break; // break this loop, read new characters from I/O
            }
            else if (type == SEQ_CORRUPTED || type == SEQ_NONE) {
            } else {
                seqs.emplace_back(type, raw.substr(rawp + p_begin, p_sz));
                seqs_cnt++;
            }
            rawp += s_end;
        } else {
            texts.push_back(raw[rawp]);
            rawp++;
        }
    }
    return seqs_cnt;
}
//
void SerialConnVT::RunParserBenchmark()
{
    FileSel fs;
    if (fs.AllFilesType().ExecuteOpen()) {
        auto filename = fs.Get();
        FileIn fin;
        if (fin.Open(filename)) {
            int64 filesz = fin.GetSize();
            char* buffer = new char[filesz + 1];
            fin.Get(buffer, filesz);
            fin.Close();
            buffer[filesz] = '\0';
            std::deque<Seq> seqs;
            if (1) {
                auto t1 = std::chrono::high_resolution_clock::now();
                size_t ns = ParseSeqs(buffer, seqs);
                auto t2 = std::chrono::high_resolution_clock::now();
                auto ts = std::chrono::duration<double>(t2 - t1).count();
                auto ps = ns / ts;
                std::string report = std::to_string(ps) + " seqs per second - Parser";
                PromptOK(report.c_str());
            }
            // render seqs
            if (1) {
                size_t ns = seqs.size();
                auto t1 = std::chrono::high_resolution_clock::now();
                RenderSeqs(seqs);
                auto t2 = std::chrono::high_resolution_clock::now();
                auto ts = std::chrono::duration<double>(t2 - t1).count();
                auto ps = ns / ts;
                //
                UpdateVScrollbar();
                UpdateHScrollbar();
                //
                std::string report = std::to_string(ps) + " seqs per second - Renderer";
                PromptOK(report.c_str());
            }
        }
    }
}
// receiver
void SerialConnVT::RxProc()
{
    std::string raw, texts; // before the loop, define these two vars firstly.
    texts.reserve(32768);
    while (!mRxShouldStop) {
        int sz = GetIo()->Available();
        if (sz < 0) {
            PostCallback([=]() {
                PromptOK(DeQtf(this->ConnName()  + ":" + t_("I/O device was disconnected")));
            });
            break;
        } else if (sz == 0) {
            std::this_thread::sleep_for(std::chrono::duration<double>(0.001));
            continue;
        }
        // read raw, not string, so, we could read NUL from the device.
        std::vector<byte> buff = GetIo()->ReadRaw(sz);
        for (size_t k = 0; k < buff.size(); ++k) {
            if (buff[k] != 0) // IGNORE NULL
                raw.push_back(buff[k]);
        }
        RefineTheInput(raw);
        //
        size_t rawp = 0;
        std::lock_guard<std::mutex> _(mLockSeqs);
        while (rawp < raw.length() && !mRxShouldStop) {
            if (IsControlSeqPrefix((uint8_t)raw[rawp])) { // is prefix of some control sequence
                if (!texts.empty()) {
                    size_t ep; // end position
                    auto s = this->TranscodeToUTF32(texts, ep);
#if 0 // discard the corrupted parts
                    texts = texts.substr(ep);
                    // add the unrecognized chars, treat them as UTF-32
                    for (size_t k = 0; k < texts.size(); ++k) {
                        s.push_back((uint8_t)texts[k]);
                    }
#endif
                    if (!s.empty()) {
                        mSeqs.emplace_back(std::move(s));
                    }
                    texts.clear();
                }
                size_t p_begin, p_sz, s_end;
                int type = IsControlSeq(raw.c_str() + rawp, p_begin, p_sz, s_end);
                if (type == SEQ_PENDING) {
                    // should we go on ?
                    if (*raw.rbegin() == '\r') { // CR will break the pending sequence
                        type = SEQ_CORRUPTED;
                    } else break; // break this loop, read new characters from I/O
                }
                else if (type == SEQ_CORRUPTED || type == SEQ_NONE) {
                    // TODO: process corrupted, or unrecognized seqence
                    LOGF("Unrecognized:%s\n", raw.substr(rawp, s_end).c_str());
                } else {
                    mSeqs.emplace_back(type, raw.substr(rawp + p_begin, p_sz));
                }
                rawp += s_end;
            } else {
                texts.push_back(raw[rawp]);
                rawp++;
            }
        }
        if (!texts.empty()) {
            size_t ep;
            auto s = this->TranscodeToUTF32(texts, ep);
            texts = texts.substr(ep);
            if (!s.empty()) {
                mSeqs.emplace_back(std::move(s));
            }
        }
        PostCallback([=]() { RenderSeqs(); });
        //
        raw = raw.substr(rawp);
    }
}
// This routine guarantee that the width of any char is integral multiple
// of mFontW.
extern int mk_wcwidth(uint32_t ucs);
int SerialConnVT::GetCharWidth(const VTChar& c) const
{
    int cx = mFontW;
    switch (c.Code()) {
    case '\t': if (1) {
        int tabsz = mFontW*mTabWidth;
        cx = tabsz - (mPx % tabsz);
    } break;
    case '\v':
    case '\n':
    case ' ':
        break;
    default: if (1) {
        cx *= mk_wcwidth(c.Code());
    } break;
    }
    return cx;
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
Point SerialConnVT::VirtualToLogic(const std::deque<VTLine>& lines, int vx, int vy, bool ignore_tail_blanks)
{
    if (vy < 0 || vy >= (int)lines.size())
        return Size(-1, -1);
    int ly = 0;
#if ENABLE_FIXED_LINE_HEIGHT == 0
    for (size_t k = 0; k < vy && k < lines.size(); ++k) {
        ly += lines[k].GetHeight();
    }
#else
	ly = mFontH * (int)lines.size();
#endif
    int lx = 0;
    int nchars = ignore_tail_blanks ?
        (int)lines[vy].size() - this->CalculateNumberOfPureBlankCharsFromEnd(lines[vy]) : (int)lines[vy].size();
    for (int k = 0; k < vx && k < nchars; ++k) {
        lx += GetCharWidth(lines[vy][k]);
    }
    return Point(lx, ly);
}
//
Point SerialConnVT::VirtualToLogic(int vx, int vy, bool ignore_tail_blanks)
{
    const VTLine* vline = this->GetVTLine(vy);
    if (!vline)
        return Point(-1, -1);
    // calculate ly
    int ly = 0;
#if ENABLE_FIXED_LINE_HEIGHT == 0
    for (size_t k = 0; k < (int)mLinesBuffer.size() && k < vy; ++k) {
        ly += mLinesBuffer[k].GetHeight();
    }
    for (int k = (int)mLinesBuffer.size(); k < vy; ++k) {
        ly += mLines[k - (int)mLinesBuffer.size()].GetHeight();
    }
#else
	ly = mFontH * vy;
#endif
    // calculate lx
    int lx = 0;
    int nchars = ignore_tail_blanks ?
        (int)vline->size() - this->CalculateNumberOfPureBlankCharsFromEnd(*vline) : (int)vline->size();
    for (int k = 0; k < vx && k < nchars; ++k) {
        lx += GetCharWidth(vline->at(k));
    }
    return Point(lx, ly);
}
//
int SerialConnVT::VirtualToLogic(const VTLine& vline, int vx, bool ignore_tail_blanks)
{
    int lx = 0;
    int nchars = ignore_tail_blanks ?
        (int)vline.size() - this->CalculateNumberOfPureBlankCharsFromEnd(vline) : (int)vline.size();
    for (int i = 0; i < vx && i < nchars; ++i) {
        lx += GetCharWidth(vline[i]);
    }
    return lx;
}
//
int SerialConnVT::GetLogicWidth(const VTLine& vline, int count, bool ignore_tail_blanks)
{
    int x = 0;
    if (count < 0)
        count = ignore_tail_blanks ?
            (int)vline.size() - this->CalculateNumberOfPureBlankCharsFromEnd(vline) : (int)vline.size();
    for (int k = 0; k < count && k < (int)vline.size(); ++k) {
        x += GetCharWidth(vline[k]);
    }
    return x;
}
//
// lx, ly - lines is virtual screen
Point SerialConnVT::LogicToVirtual(const std::deque<VTLine>& lines, int lx, int ly, int& px, int& next_px,
    int& py, int& next_py, bool ignore_tail_blanks)
{
    if (lx < 0)
        lx = 0;
    if (ly < 0)
        ly = 0; // 0~
    int vy = (int)lines.size() - 1;
    py = 0;
    next_py = 0;
#if ENABLE_FIXED_LINE_HEIGHT == 0
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
#else
	py = ly;
	next_py = py + mFontH;
	vy = ly / mFontH;
	if (vy >= (int)lines.size())
		vy = -1;
#endif
    int nchars = ignore_tail_blanks ?
        (int)lines[vy].size() - this->CalculateNumberOfPureBlankCharsFromEnd(lines[vy]) : (int)lines[vy].size();
    int vx = nchars == 0 ? 0 : (int)nchars - 1;
    px = 0, next_px = 0;
    for (int i = 0; i < nchars; ++i) {
        int vchar_sz = GetCharWidth(lines[vy][i]);
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
    int& py, int& next_py, bool ignore_tail_blanks)
{
    if (lx < 0)
        lx = 0;
    if (ly < 0)
        ly = 0; // 0~
    int vy = -1;
    py = 0;
    next_py = 0;
#if ENABLE_FIXED_LINE_HEIGHT == 0
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
#else
	py = mFontH * vy;
	next_py = py + mFontH;
	vy = ly / mFontH;
	if (vy >= (int)(mLinesBuffer.size() + mLines.size()))
		vy = (int)(mLinesBuffer.size() + mLines.size()) - 1;
#endif
    if (vy < 0)
        return Point(-1, -1); // Error, what's wrong? Because lines and linesbuffer all all empty.
    //
    const VTLine* vline = this->GetVTLine(vy);
    int nchars = ignore_tail_blanks ?
        (int)vline->size() - this->CalculateNumberOfPureBlankCharsFromEnd(*vline) : (int)vline->size();
    int vx = nchars == 0 ? 0 : (int)nchars - 1;
    px = 0; next_px = 0;
    for (int k = 0; k < nchars; ++k) {
        int vchar_sz = GetCharWidth(vline->at(k));
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
int SerialConnVT::LogicToVirtual(const VTLine& vline, int lx, int& px, int& next_px, bool ignore_tail_blanks)
{
    if (lx < 0)
        lx = 0; // 0~
    int nchars = ignore_tail_blanks ?
        (int)vline.size() - this->CalculateNumberOfPureBlankCharsFromEnd(vline) : (int)vline.size();
    int vx = nchars == 0 ? 0 : (int)nchars - 1;
    px = 0; next_px = 0;
    for (int i = 0; i < nchars; ++i) {
        int vchar_sz = GetCharWidth(vline[i]);
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
    int nchars = (int)vline.size() - CalculateNumberOfBlankCharsFromEnd(vline);
    VTLine buff = VTLine(nchars).SetHeight(vline.GetHeight());
    for (int i = 0; i < nchars; ++i) {
        buff[i] = vline[i];
    }
    mLinesBuffer.push_back(std::move(buff));
    if (mLinesBuffer.size() > mMaxLinesBufferSize) {
		mLinesBuffer.erase(mLinesBuffer.begin());
#if 1
		// we should move the selection span up
		if (mSelectionSpan.Y0 > 0) {
			mSelectionSpan.Y0 -= 1;
		} else {
			mSelectionSpan.X0 = 0;
		}
		if (mSelectionSpan.Y1 > 0) mSelectionSpan.Y1 -= 1;
		else {
			mSelectionSpan.Valid = false;
		}
#endif
    }
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
bool SerialConnVT::ProcessOverflowChars()
{
	bool overflow = false;
	if (mVy < 0) mVy = 0;
    if (mVy < (int)mLines.size()) {
	    int dx = mVx - (int)mLines[mVy].size() + 1;
	    if (dx > 0) {
	        mLines[mVy].insert(mLines[mVy].end(), dx, mBlankChar);
	        overflow = true;
	    }
    } else {
        // set vy to bottom if it is wrong
        mVy = (int)mLines.size()-1;
    }
    return overflow;
}
// If the scrolling region was set, we should ignore those lines out of region
bool SerialConnVT::ProcessOverflowChars(const struct Seq&)
{
	return ProcessOverflowChars();
}
bool SerialConnVT::ProcessOverflowLines()
{
	int top = mScrollingRegion.Top;
	int bot = mScrollingRegion.Bottom;
	if (bot < 0) bot = (int)mLines.size()-1;
	if (mVy < 0) mVy = 0;
	int yn = mVy - bot;
	// I tested mintty, we buffer lines like that.
	if (yn == 1) {
		if (top == 0) {
		    this->PushToLinesBufferAndCheck(std::move(mLines[top]));
		}
		for (size_t k = top+1; k <= bot; ++k) {
		    std::swap(mLines[k-1], mLines[k]);
		}
		VTLine& vline = mLines[bot];
		for (size_t k = 0; k < vline.size(); ++k) {
		    vline[k] = mBlankChar;
		}
		// fix
		mVy = bot;
		//
		return true;
	}
	if (mVy >= (int)mLines.size()) {
		mVy = (int)mLines.size()-1;
	}
	return false;
}
bool SerialConnVT::ProcessOverflowLines(const struct Seq&)
{
	return ProcessOverflowLines();
}
//
uint32_t SerialConnVT::RemapCharacter(uint32_t uc, int charset)
{
	(void)charset;
	//
    return uc;
}
void SerialConnVT::RenderText(const std::vector<uint32_t>& s)
{
    Size csz = GetConsoleSize();
    int cs = 0;
    for (size_t k = 0; k < s.size(); ++k) {
        VTChar chr = RemapCharacter(s[k], mCharset);
        chr.SetStyle(mStyle);
        // Unfortunately, UPP does not support complete UNICODE, support UCS-16 instead. So
        // we should ignore those out of range
        if (chr.Code() > 0xffff)
            chr.SetCode('?'); // replace chr with ?
        if (mWrapLine) {
	        if (mVx >= csz.cx || mPx >= mFontW*csz.cx) { // wrap line
	            mLines[mVy].HasSuccessiveLines(true);
	            // insert or move to next line.
	            mVy++; mVx = 0;
	            if (this->ProcessOverflowLines()) {
		            this->UpdateVScrollbar();
		            this->UpdatePresentationPos();
	            }
	        }
        }
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
        if (mVx >= (int)vline.size()) {
            // extend vline
            vline.insert(vline.end(), 8, mBlankChar);
        }
        if (mWrapLine) {
            UpdatePresentationPos(0x1);
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
    if (next_px != 0)
        mSelectionSpan.X0 = (lx - px) > (next_px - px) / 2 ? vpos.x + 1 : vpos.x;
    else
        mSelectionSpan.X0 = 0;
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
            while (bx < (int)vline->size() && vline->at(bx).Code() != ' ' && vline->at(bx).Code() != '\n') {
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
        mSelectionSpan.Y0 = vpos.y;
        mSelectionSpan.Y1 = vpos.y;
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
    text.Replace("\r\n", "\n");
    GetIo()->Write(text.ToStd());
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
        mSelectionSpan.Y1 = (int)(mLinesBuffer.size() + mLines.size());
        mSelectionSpan.Valid = true;
        //
        Refresh();
    }
}

void SerialConnVT::RightUp(Point, dword key)
{
    MenuBar::Execute(WhenBar);
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
        case K_DELETE:
            d.push_back(0x7f);
            break;
        default:
            break;
        }
    } else if ((flags & (K_CTRL | K_SHIFT)) == (K_CTRL | K_SHIFT)) { // CTRL+SHIFT+
        switch (key) {
        case K_2: // CTRL+SHIFT+2 = CTRL+@
            d.push_back(0x00); // NUL
            break;
        case K_6: // CTRL+SHIFT+6 = CTRL+^
            d.push_back(0x1e); // RS
            break;
        case K_GRAVE: // CTRL+SHIFT+`=CTRL+~
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
            case K_SPACE:
                d.push_back(0x00); // CTRL+Space
                break;
            case K_LBRACKET: // CTRL+[
                d.push_back(0x1b);
                break;
            case K_SLASH: // CTRL+/
                d.push_back(0x1c);
                break;
            case K_RBRACKET: // CTRL+]
                d.push_back(0x1d);
                break;
            }
        }
    }
    if (!d.empty()) {
        GetIo()->Write(d);
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
    GetIo()->Write(GetCodec()->TranscodeFromUTF32(cc));
    //
    return true;
}

bool SerialConnVT::Key(dword key, int)
{
    bool processed = false;
    //
    if (MenuBar::Scan(WhenBar, key)) return true;
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
            // key is a character [WCHAR]
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
int SerialConnVT::CalculateNumberOfBlankLinesFromEnd(const std::deque<VTLine>& lines) const
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
//
int SerialConnVT::CalculateNumberOfPureBlankCharsFromEnd(const VTLine& vline) const
{
    int cn = 0;
    size_t sz = vline.size();
    while (sz--) {
        if (vline[sz].Code() == ' ' && vline[sz].FontStyle() == VTStyle::eVisible)
            cn++;
        else
            break;
    }
    return cn;
}
//
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
    int blankcnt = std::min(CalculateNumberOfBlankLinesFromEnd(mLines), (int)mLines.size() - mVy);
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
#if 0
            while (overflow_cnt--)
                vline.pop_back();
#else
            // only strip tail blanks
            int blanks_cnt = this->CalculateNumberOfBlankCharsFromEnd(vline);
            int k = 0;
            while (k < overflow_cnt && k < blanks_cnt) {
                k++;
                vline.pop_back();
            }
#endif
        }
    }
    // extend or shrink the virtual screen
    if (mLines.size() < csz.cy) {
        ExtendVirtualScreen(csz.cx, csz.cy);
        //--------------------------------------------------------------------------------------
    } else {
        ShrinkVirtualScreen(csz.cx, csz.cy);
    }
    ProcessOverflowChars(Seq());
    // screen size was changed
    mScrollingRegion.Top = 0; mScrollingRegion.Bottom = -1;
    //
    WhenSizeChanged(GetConsoleSize());
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
    int nlines = (int)(mLinesBuffer.size() + mLines.size()) - this->CalculateNumberOfBlankLinesFromEnd(mLines);
    std::vector<std::string> out;
    bool has_successive = false;
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
        has_successive = vline->HasSuccessiveLines();
        out.push_back(sel);
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
        if (has_successive) {
            *out.rbegin() += sel;
        } else out.push_back(sel);
        has_successive = vline->HasSuccessiveLines();
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
        if (has_successive) {
            *out.rbegin() += sel;
        } else out.push_back(sel);
        has_successive = vline->HasSuccessiveLines();
    }
    return out;
}
//
String SerialConnVT::GetSelectedText() const
{
    String out;
    auto lines = GetSelection();
    for (size_t k = 0; k < lines.size(); ++k) {
        if (k != 0)
            out += '\n';
        // strip successive blanks from tail
        size_t n = 0;
        while (n < lines[k].size()) {
            uint32_t c = lines[k][lines[k].size() - 1 - n];
            if (c == ' ')
                n++;
            else
                break;
        }
        out += lines[k].substr(0, lines[k].size() - n);
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
    if (!mShowCursor) return;
    
    int px = mPx - mSbH.Get();
    int py = mPy + (mSbV.GetTotal() - mSbV.Get() - mSbV.GetPage());
#if ENABLE_FIXED_LINE_HEIGHT == 0
    // do nothing here
#else
    // align
	py = py / mFontH * mFontH;
#endif
    Size usz = GetSize();
    if (px >= 0 && py < usz.cx && py >= 0 && py < usz.cy) {
        // a visible char.
        if (mVy < (int)mLines.size() && mVx < (int)mLines[mVy].size()
            && mLines[mVy][mVx].Code() != ' ' && mLines[mVy][mVx].Code() != '\t') {
            const VTChar& cc = mLines[mVy][mVx];
            draw.DrawRect(px, py, GetCharWidth(cc), mFontH, Color(0, 255, 0));
            this->DrawVTChar(draw, px, py, cc, mFont, \
                mColorTbl.GetColor(VTColorTable::kColorId_Paper));
        } else {
            draw.DrawRect(px, py, mFontW, mFontH, Color(0, 255, 0));
        }
    }
}

int SerialConnVT::GetVTLinesHeight(const std::vector<VTLine>& lines) const
{
#if ENABLE_FIXED_LINE_HEIGHT == 1
	return mFontH * (int)lines.size();
#else
    int linesz = 0;
    for (size_t k = 0; k < lines.size(); ++k) {
        linesz += lines[k].GetHeight();
    }
    return linesz;
#endif
}
int SerialConnVT::GetVTLinesHeight(const std::deque<VTLine>& lines) const
{
#if ENABLE_FIXED_LINE_HEIGHT == 1
	return mFontH * (int)lines.size();
#else
    int linesz = 0;
    for (auto it = lines.begin(); it != lines.end(); ++it) {
        linesz += it->GetHeight();
    }
    return linesz;
#endif
}

void SerialConnVT::UseStyle(const VTChar& c, Upp::Font& font, Upp::Color& fg_color, Upp::Color& bg_color,
    bool& blink, bool& visible)
{
    uint16_t fgcolor_id, bgcolor_id;
    c.UseStyle(font, fgcolor_id, bgcolor_id, blink, visible);
    fg_color = mColorTbl.GetColor(fgcolor_id);
    bg_color = mColorTbl.GetColor(bgcolor_id);
}

static inline bool _IsBlank(const VTChar& c)
{
	return c.Code() <= ' ';
}

void SerialConnVT::DrawVTLine(Draw& draw, const VTLine& vline,
    int vx, int vy, /*! absolute position of data */
    int lxoff, int lyoff)
{
    Size csz = GetConsoleSize();
    int x = lxoff, y = lyoff, i = 0, r_margin = mFontW*csz.cx;
    bool tail_selected = IsCharInSelectionSpan((int)vline.size() - 1, vy);
    bool line_selected = IsCharInSelectionSpan(0, vy) && tail_selected; // line was selected.
    // draw blank chars
    int abc_cnt = (int)vline.size() - this->CalculateNumberOfPureBlankCharsFromEnd(vline);
    // style
    const Color& paper_color = mColorTbl.GetColor(VTColorTable::kColorId_Paper);
    Color bg_color, fg_color; bool blink, visible;
    for (i = vx; i < (int)vline.size() && x < r_margin; ++i) {
        int vchar_cx = GetCharWidth(vline[i]);
        UseStyle(vline[i], mFont, fg_color, bg_color, blink, visible);
        bool is_selected = line_selected ? true : IsCharInSelectionSpan(i, vy);
        if (is_selected) {
            std::swap(bg_color, fg_color);
        }
        if (blink) {
            if (mBlinkSignal) {
                draw.DrawRect(x, y, vchar_cx, vline.GetHeight(), paper_color);
            } else {
                if (bg_color != paper_color) {
                    draw.DrawRect(x, y, vchar_cx, vline.GetHeight(), bg_color);
                }
                if (fg_color != bg_color) {
                    if (!_IsBlank(vline[i]) && visible) {
                        DrawVTChar(draw, x, y, vline[i], mFont, fg_color);
                    }
                }
            }
        } else {
            if (bg_color != paper_color) {
                draw.DrawRect(x, y, vchar_cx, vline.GetHeight(), bg_color);
            }
            if (fg_color != bg_color) {
                if (!_IsBlank(vline[i]) && visible) {
                    DrawVTChar(draw, x, y, vline[i], mFont, fg_color);
                }
            }
        }
        x += vchar_cx;
        if (is_selected) { // restore color
            std::swap(bg_color, fg_color);
        }
    }
}

void SerialConnVT::UpdateDataPos(int flags)
{
    int px, py, next_px, next_py;
    if ((flags & 0x3) == 0x3) {
        Point vpos = this->LogicToVirtual(mLines, mPx, mPy, px, next_px, py, next_py, false);
        mVx = vpos.x;
        mVy = vpos.y;
    }
    else if (flags & 0x1) {
        mVx = this->LogicToVirtual(mLines[mVy], mPx, px, next_px, false);
    }
    else if (flags & 0x2) {
        mVy = this->LogicToVirtual(mLines, 0, mPy, px, next_px, py, next_py, false).y;
    }
}

void SerialConnVT::UpdatePresentationPos(int flags)
{
    if (mLines.empty()) return;
    if (flags & 0x2) {
#if ENABLE_FIXED_LINE_HEIGHT == 0
        // cache to avoid multiple calculating
        int buff_height = 0;
	    int posl_height = 0;
	    for (size_t k = 0; k < mLinesBuffer.size(); ++k) {
	        buff_height += mLinesBuffer[k].GetHeight();
	    }
	    posl_height = buff_height;
	    for (size_t k = 0; k < mVy; ++k) {
	        posl_height += mLines[k].GetHeight();
	    }
        mPy = posl_height - buff_height;
#else
		mPy = mFontH * mVy;
#endif
    }
    if (flags & 0x1) {
        int x0 = this->VirtualToLogic(mLines[mVy], mVx, false);
        int x1 = x0 + GetCharWidth(mLines[mVy][mVx]);
        // If mPx is out of range [x0,x1], update it.
        if (mPx <= x0 || mPx >= x1) {
            mPx = x0;
        }
    }
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
#if ENABLE_H_SCROLLBAR
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
        int nchars = (int)vline.size() - this->CalculateNumberOfBlankCharsFromEnd(vline);
        int linesz = this->GetLogicWidth(vline, nchars);
        if (linesz > longest_linesz) {
            longest_linesz = linesz;
        }
    }
    for (int i = std::max(0, vpos.y - (int)mLinesBuffer.size()); i < (int)mLines.size() && lyoff < usz.cy; ++i) {
        const VTLine& vline = mLines[i];
        lyoff += vline.GetHeight();
        int nchars = (int)vline.size() - this->CalculateNumberOfBlankCharsFromEnd(vline);
        int linesz = this->GetLogicWidth(vline, nchars);
        if (linesz > longest_linesz) {
            longest_linesz = linesz;
        }
    }
    // get the longest line
    VTLine* vline = GetVTLine((int)mLinesBuffer.size() + mVy);
    if (vline) {
        longest_linesz = std::max(this->GetLogicWidth(*vline, mVx, false), longest_linesz);
    }
    mSbH.SetTotal(longest_linesz);
#endif
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
#if ENABLE_FIXED_LINE_HEIGHT == 0
    int lyoff = py - ly;
#else
	int lyoff = 0;
#endif
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
