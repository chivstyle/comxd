//
// (c) 2020 chiv
//
#include "resource.h"
#include "SerialConnVT.h"
#include "ConnFactory.h"
// register
using namespace Upp;

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
    mSbV.SetLine(mFontH); mSbV.Set(0); mSbV.SetTotal(0);
    AddFrame(mSbV);
    // Install handlers
    InstallControlSeqHandlers();
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
    std::string pattern;
    while (!mRxShouldStop) {
        size_t sz = mSerial->available();
        if (sz) {
            std::string pretty_buff;
            std::vector<uint8_t> buff;
            GetSerial()->read(buff, sz);
            for (size_t k = 0; k < buff.size(); ++k) {
                if (pending) {
#if 0 // Support VT CANCEL(0x18)
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

Size SerialConnVT::GetConsoleSize() const
{
    Size sz = GetSize();
    // monospace
    ASSERT(mFont.GetFaceInfo() & Font::FIXEDPITCH);
    
    return Upp::Size(sz.cx / mFontW, sz.cy / mFontH);
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
void SerialConnVT::InstallControlSeqHandlers()
{
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
void SerialConnVT::RenderText(const std::string& seq, int seq_type)
{
    std::lock_guard<std::mutex> _(mLinesLock);
    Size csz = GetConsoleSize();
    if (seq[0] != 0x1b) { // Not VT control seq
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
			String text2 = Upp::ReadClipboardText();
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

void SerialConnVT::Render(Upp::Draw& draw)
{
    // draw background
    draw.DrawRect(GetRect(), mPaperColor);
    // set total
    int nlines = (int)mLinesBuffer.size() + \
        (int)mLines.size();// - this->CalculateNumberOfBlankLinesFromEnd(mLines);
    mSbV.SetTotal(mFontH * nlines);
    // draw VT chars
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

void SerialConnVT::Render(Upp::Draw& draw, const std::vector<VTLine>& vlines)
{
    Size csz = GetConsoleSize();
    //
    int x = 0, y = 0;
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
                    draw.DrawRect(x, y, mFontW, mFontH, mPaperColor);
                } else {
                    if (mBgColor != mDefaultBgColor) {
                        draw.DrawRect(x, y, mFontW, mFontH, mBgColor);
                    }
                    if (mFgColor != mBgColor) {
                        if (buff[0] != ' ' && mVisible) {
                            draw.DrawText(x, y, buff, mFont, mFgColor);
                        }
                    }
                }
            } else {
                if (mBgColor != mDefaultBgColor) {
                    draw.DrawRect(x, y, mFontW, mFontH, mBgColor);
                }
                if (mFgColor != mBgColor) {
                    if (buff[0] != ' ' && mVisible) {
                        draw.DrawText(x, y, buff, mFont, mFgColor);
                    }
                }
            }
            x += mFontW;
            if (is_selected) {
                std::swap(mBgColor, mFgColor);
            }
        }
        // apply attrs for those lines longer than screen width
        for (size_t i = csz.cx; i < vline.size(); ++i) {
            vline[i].ApplyAttrs();
        }
        // move to next line
        x = 0;
        y += mFontH;
    }
}

