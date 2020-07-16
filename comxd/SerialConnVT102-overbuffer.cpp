//
// (c) 2020 chiv
//
#include "resource.h"
#include "SerialConnVT102.h"
#include "VTxControlSeq.h"
//
using namespace Upp;
/// VT102 Control Codes
/// please read [VT102 Manual](https://vt100.net/docs/vt102-ug/appendixc.html)
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
    , mRealLines(0)
    , mRealX(0)
    , mScrollToEnd(true)
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
    mCurrAttrFunc = [&]() { mFgColor = mDefaultFgColor; };
    // enable scroll bar
    mSbV.WhenScroll = [=]() {
        mScrollToEnd = (mSbV.Get() + GetSize().cy) / mFontH == (mSbV.GetTotal() / mFontH);
        Refresh();
    };
    mSbV.SetLine(mFontH); mSbV.Set(0); mSbV.SetTotal(0);
    AddFrame(mSbV);
    // Install VTx handlers
    InstallVTxControlSeqHandlers();
    //
    mRxThr = std::thread([=]() { RxProc(); });
}

SerialConnVT102::~SerialConnVT102()
{
    mRxShouldStop = true;
    if (mRxThr.joinable()) {
        mRxThr.join();
    }
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
            std::string buff = GetSerial()->read(sz);
            for (size_t k = 0; k < buff.size(); ++k) {
                if (pending) {
                    pattern.push_back(buff[k]);
                    int ret = IsVTxControlSeq(pattern);
                    if (ret == 2) { // bingo
                        // found it.
                        Upp::PostCallback([=]() { RenderText("\x1b" + pattern); });
                        //
                        pending = false;
                        pattern = "";
                    } else if (ret == 0) { // no, it's not
                        Upp::PostCallback([=]() { RenderText(pattern); });
                        pending = false;
                        pattern = "";
                    }
                } else if (buff[k] == 0x1b) {
                    // come across a 0x1b, render the pretty_buff
                    if (!pretty_buff.empty()) {
                        Upp::PostCallback([=]() { RenderText(pretty_buff); });
                    }
                    pretty_buff = "";
                    pending = true; // continue
                } else if (buff[k] == '\r') continue; else {
                    pretty_buff.push_back(buff[k]);
                }
            }
            // render the pretty_buff
            if (!pretty_buff.empty()) {
                Upp::PostCallback([=]() { RenderText(pretty_buff); });
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
void SerialConnVT102::InstallVTxControlSeqHandlers()
{
    // erasing
    mCtrlHandlers["[K"] = [=]() { // erase in line, cursor to end of line
        Size csz = GetConsoleSize();
        for (int i = mCursorX; i < csz.cx; ++i) {
            mLines[mCursorY][i] = ' ';
        }
    };
    mCtrlHandlers["[0K"] = mCtrlHandlers["[K"];
    mCtrlHandlers["[1K"] = [=]() { // erase in line, beginning of line to cursor
        Size csz = GetConsoleSize();
        for (int i = 0; i < mCursorX; ++i) {
            mLines[mCursorY][i] = ' ';
        }
    };
    mCtrlHandlers["[2K"] = [=]() { // erase in line, entire line containing cursor
        Size csz = GetConsoleSize();
        mLines[mCursorY] = VTxLine(csz.cx, ' ');
    };
    mCtrlHandlers["[J"] = [=]() { // erase in display, cursor to end of screen
        Size csz = GetConsoleSize();
        for (int i = mCursorX; i < csz.cx; ++i) {
            mLines[mCursorY][i] = ' ';
        }
        for (int i = mCursorY+1; i < csz.cy; ++i) {
            mLines[i] = VTxLine(csz.cx, ' ');
        }
    };
    mCtrlHandlers["[0J"] = mCtrlHandlers["[J"];
    mCtrlHandlers["[1J"] = [=]() { // erase in display, beginning of screen to cursor
        Size csz = GetConsoleSize();
        for (int i = 0; i < mCursorX; ++i) {
            mLines[mCursorY][i] = ' ';
        }
        for (int i = 0; i < mCursorY; ++i) {
            mLines[i] = VTxLine(csz.cx, ' ');
        }
    };
    mCtrlHandlers["[2J"] = [=]() { // erase in display, entire screen
        Size csz = GetConsoleSize();
        for (size_t k = 0; k < mLines.size(); ++k) {
            mLines[k] = VTxLine(csz.cx, ' ');
        }
    };
    // cursor
    mCtrlHandlers["[H"] = [=]() { // cursor position (home)
        // home
        mScrollToEnd = true;
        mCursorX = 0;
        mCursorY = 0;
    };
}
//
void SerialConnVT102::ProcessVTxCursorKeyCodes(const std::string& seq)
{
    int n = atoi(seq.substr(1, seq.length() - 2).c_str());
    Size csz = GetConsoleSize();
    switch (*seq.rbegin()) {
    case 'A':
        break;
    case 'B':
        break;
    case 'C':if (1) {
        mCursorX += n;
        mCursorX = std::min(mCursorX, csz.cx-1);
    } break;
    case 'D':if (1) {
        mCursorX -= n;
        mCursorX = std::max(mCursorX, 0);
    } break;
    default:break;
    }
}
//
void SerialConnVT102::ProcessVTxControlSeq(const std::string& seq)
{
    auto it = mCtrlHandlers.find(seq);
    if (it != mCtrlHandlers.end()) {
        it->second();
    } else {
        if (IsVTxCursorKeyCodes(seq)) {
            ProcessVTxCursorKeyCodes(seq);
        }
    }
}
//
void SerialConnVT102::ProcessAsciiControlChar(unsigned char cc)
{
    switch (cc) {
    case '\n':
        mRealLines++;
        //
        mCursorY += 1;
        mCursorX = 0;
        break;
    case 0x08: // backspace
        mCursorX -= 1;
        break;
    case 0x07: // bell
        mCursorX = mRealX;
        break;
    }
}

void SerialConnVT102::RenderText(const std::string& seq)
{
    std::lock_guard<std::mutex> _(mLinesLock);
    Size csz = GetConsoleSize();
    if (seq[0] != 0x1b) { // Not VT102 control seq
        for (size_t k = 0; k < seq.size(); ++k) {
            VTxChar chr = seq[k];
            chr.SetAttrFun(mCurrAttrFunc);
            if (isprint(chr)) {
                VTxLine& vline = mLines[mCursorY];
                if (mCursorX < vline.size()) {
                    vline[mCursorX++] = chr;
                } else {
                    // extend this line, we'll record all chars
                    vline.push_back(chr);
                }
            } else {
                ProcessAsciiControlChar(chr);
            }
            if (mCursorY >= csz.cy) { // scroll
                size_t cn = mCursorY - csz.cy + 1;
                for (size_t k = 0; k < cn; ++k) {
                    mLinesBuffer.push_back(mLines[0]);
                    mLines.erase(mLines.begin());
                    mLines.push_back(VTxLine(csz.cx, ' '));
                }
                mCursorY = csz.cy - 1; // move to last line
                mRealLines = csz.cy - 1;
            }
        }
    } else {
        ProcessVTxControlSeq(seq.substr(1));
    }
    mRealX = mCursorX;
}

void SerialConnVT102::Paint(Upp::Draw& draw)
{
    // draw background
    draw.DrawRect(this->GetRect(), Color(0, 0, 0));
    //
    Render(draw);
}

void SerialConnVT102::MouseWheel(Point, int zdelta, dword)
{
	mSbV.Wheel(zdelta);
}

void SerialConnVT102::LeftUp(Point p, dword)
{
}

void SerialConnVT102::LeftDown(Point p, dword)
{
}
//----------------------------------------------
bool SerialConnVT102::ProcessKeyDown(dword key, dword flags)
{
    std::string text = std::to_string(K_DELETE);
    if ((flags & (K_CTRL | K_ALT | K_SHIFT)) == 0) {
        std::vector<uint8_t> d;
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
            mCursorX = 0;
            d.push_back(0x1b);
            d.push_back('[');
            d.push_back('A');
            break;
        case K_DOWN:
            mCursorX = 0;
            d.push_back(0x1b);
            d.push_back('[');
            d.push_back('B');
            break;
        default:break;
        }
        if (!d.empty()) {
            GetSerial()->write(d);
            return true;
        }
    }
    return false;
}
bool SerialConnVT102::ProcessKeyUp(dword key, dword flags)
{
    if (flags & K_CTRL) {
        std::vector<uint8_t> d;
        switch (key) {
        case K_C:
            d.push_back(3);
            break;
        }
        if (!d.empty()) {
            GetSerial()->write(d);
            return true;
        }
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
	    dword flags = K_CTRL | K_ALT | K_SHIFT;
	    dword d_key = key & ~(flags | K_KEYUP);
	    flags = key & flags;
	    if ((key & K_KEYUP) == 0) { // key down, most ascii
	        if (key == 0x09) { // Tab
	            mCursorX = 0;
	        }
	        if (d_key >= 0 && d_key <= 0x7f) {
	            GetSerial()->write((uint8_t*)&d_key, 1);
	            processed = true;
	        }
	    }
	}
	if (processed) {
	    mScrollToEnd = true;
	}
	return processed;
}

void SerialConnVT102::Layout()
{
	Size csz = GetConsoleSize();
	mSbV.SetPage(GetSize().cy);
	if (mLines.size() < csz.cy) {
	    size_t cn = csz.cy - mLines.size();
	    size_t ln = 0;
	    while (ln < cn && ln < mLinesBuffer.size()) {
	        VTxLine vline = mLinesBuffer[mLinesBuffer.size()-1];
	        mLinesBuffer.pop_back();
	        for (size_t i = vline.size(); i < csz.cx; ++i) { // padding with blanks
	            vline.push_back(' ');
	        }
	        mLines.insert(mLines.begin(), vline);
	        mCursorY++;
	        ln++;
	        mRealLines++;
	    }
	    for (size_t k = ln; k < cn; ++k) {
	        mLines.push_back(VTxLine(csz.cx, ' '));
	    }
	} else {
	    size_t blank_lines = mLines.size() - mRealLines - 1;
	    size_t cn = mLines.size() - csz.cy;
	    size_t ln = 0;
	    // remove blank lines
	    for (size_t k = 0; k < cn && k < blank_lines; ++k) {
	        mLines.pop_back();
	        ln++;
	    }
	    // remove from header
	    for (size_t k = ln; k < cn; ++k) {
	        mLinesBuffer.push_back(mLines[0]);
	        mLines.erase(mLines.begin());
	        mRealLines--;
	        mCursorY--;
	    }
	}
	mScrollToEnd = true;
}

std::vector<SerialConnVT102::VTxLine> SerialConnVT102::GetBufferLines(size_t p, int& y)
{
    Size sz = GetConsoleSize(); y = -1;
    std::vector<VTxLine> out(sz.cy);
    size_t ln = 0;
    for (size_t k = p; k < mLinesBuffer.size() && ln < sz.cy; ++k) {
        VTxLine vline(sz.cx, ' ');
        const VTxLine& line = mLinesBuffer[k];
        for (size_t i = 0; i < line.size() && i < vline.size(); ++i) {
            vline[i] = line[i];
        }
        out[ln++] = vline;
    }
    y = mCursorY + ln;
    if (y >= sz.cy) y = -1;
    size_t bn = sz.cy - ln;
    for (size_t k = 0; k < bn; ++k) {
        out[ln++] = mLines[k];
    }
    return out;
}

void SerialConnVT102::Render(Upp::Draw& draw, const std::vector<VTxLine>& vlines)
{
    Size csz = GetConsoleSize();
    int x = 0, y = 0;
    for (size_t k = 0; k < vlines.size(); ++k) {
        const VTxLine& vline = vlines[k];
        for (size_t i = 0; i < vline.size() && i < csz.cx; ++i) {
            vline[i].ApplyAttr();
            const char buff[2] = {vline[i], '\0'};
            //draw.DrawRect(x, y, mFontW, mFontH, mBgColor);
            draw.DrawText(x, y, buff, mFont, mFgColor);
            x += mFontW;
        }
        for (size_t i = csz.cx; i < vline.size(); ++i) {
            vline[i].ApplyAttr();
        }
        // move to next line
        x = 0;
        y += mFontH;
    }
}

void SerialConnVT102::Render(Upp::Draw& draw)
{
    // draw background
    draw.DrawRect(GetRect(), mPaperColor);
    // set total
    mSbV.SetTotal(mFontH*(mLinesBuffer.size() + mLines.size()));
    // draw VTx chars
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

