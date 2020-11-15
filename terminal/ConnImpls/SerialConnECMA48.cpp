//
// (c) 2020 chiv
//
#include "SerialConnECMA48.h"
#include "ECMA48ControlSeq.h"
#include "ConnFactory.h"
// We implement this as the base-class of Xterm, we'll provide ecma48 in future.
// REGISTER_CONN_INSTANCE("ECMA-48", SerialConnECMA48);

using namespace Upp;
//----------------------------------------------------------------------------------------------
// What's presentation component? What's data presentation?
//
//----------------------------------------------------------------------------------------------
SerialConnECMA48::SerialConnECMA48(std::shared_ptr<SerialIo> serial)
    : Superclass(serial)
{
    InstallEcma48Functions();
}
//
SerialConnECMA48::~SerialConnECMA48()
{
}
//
void SerialConnECMA48::ProcessSGR(int attr_code)
{
    switch (attr_code) {
    case 0:
        mStyle = VTStyle();
        break;
    case 1:
        mStyle.FontStyle |= VTStyle::eBold;
        break;
    case 2:
        mStyle.FontStyle |= ~VTStyle::eBold;
        break;
    case 3:
        mStyle.FontStyle |= VTStyle::eItalic;
        break;
    case 4:
        mStyle.FontStyle |= VTStyle::eUnderline;
        break;
    case 5:
    case 6: // rapidly blinking, No, we treat is as 5
        mStyle.FontStyle |= VTStyle::eBlink;
        break;
    case 7:
        std::swap(mStyle.FgColorId, mStyle.BgColorId);
        break;
    case 8: // conceal
        mStyle.FontStyle |= VTStyle::eVisible;
        break;
    case 9: // crossed-out
        mStyle.FontStyle |= VTStyle::eStrikeout;
        break;
    case 10: // fonts
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20: // Fraktur
    case 21: // double underlined
    case 22: // normal color, normal intensity
        mStyle.FontStyle &= VTStyle::eBold;
        mStyle.FgColorId = VTColorTable::kColorId_Texts;
        break;
    case 23: // Not italicized, not fraktur
        mStyle.FontStyle &= VTStyle::eItalic;
        break;
    case 24: // not underline
        mStyle.FontStyle &= VTStyle::eUnderline;
        break;
    case 25: // steady, no blinking
        mStyle.FontStyle &= VTStyle::eBlink;
        break;
    case 26: // reserved for proportional spacing as specified in CCITT recommendation T6.1
    case 27: // positive image
        break;
    case 28: // revealed characters
        mStyle.FontStyle |= VTStyle::eVisible;
        break;
    case 29: // not crossed
        mStyle.FontStyle &= ~VTStyle::eStrikeout;
        break;
    case 30: // black
        mStyle.FgColorId = VTColorTable::kColorId_Black;
        break;
    case 31: // red
        mStyle.FgColorId = VTColorTable::kColorId_Red;
        break;
    case 32: // green
        mStyle.FgColorId = VTColorTable::kColorId_Green;
        break;
    case 33: // yellow
        mStyle.FgColorId = VTColorTable::kColorId_Yellow;
        break;
    case 34: // blue
        mStyle.FgColorId = VTColorTable::kColorId_Blue;
        break;
    case 35: // magenta
        mStyle.FgColorId = VTColorTable::kColorId_Magenta;
        break;
    case 36: // cyan
        mStyle.FgColorId = VTColorTable::kColorId_Cyan;
        break;
    case 37: // white
        mStyle.FgColorId = VTColorTable::kColorId_White;
        break;
    case 38: // (reserved for future standardization; intended for setting character foreground colour as specified in
             // ISO 8613-6 [CCITT Recommendation T.416])
        break;
    case 39: // default color
        mStyle.FgColorId = VTColorTable::kColorId_Texts;
        break;
    case 40: // black background
        mStyle.BgColorId = VTColorTable::kColorId_Black;
        break;
    case 41: // red bg
        mStyle.BgColorId = VTColorTable::kColorId_Red;
        break;
    case 42: // green bg
        mStyle.BgColorId = VTColorTable::kColorId_Green;
        break;
    case 43: // yellow bg
        mStyle.BgColorId = VTColorTable::kColorId_Yellow;
        break;
    case 44: // blue bg
        mStyle.BgColorId = VTColorTable::kColorId_Blue;
        break;
    case 45: // magenta bg
        mStyle.BgColorId = VTColorTable::kColorId_Magenta;
        break;
    case 46: // cyan bg
        mStyle.BgColorId = VTColorTable::kColorId_Cyan;
        break;
    case 47: // white bg
        mStyle.BgColorId = VTColorTable::kColorId_White;
        break;
    case 48: // (reserved for future standardization; intended for setting character foreground colour as specified in
             // ISO 8613-6 [CCITT Recommendation T.416])
        break;
    case 49: // default color bg
        mStyle.BgColorId = VTColorTable::kColorId_Paper;
        break;
    case 50: // (reserved for cancelling the effect of the rendering aspect established by parameter value 26)
        break;
    case 51: // framed
    case 52: // encircled
    case 53: // overlined
    case 54: // not framed, not encircled
    case 55: // not overlined
        break;
    case 56: // reserved for future
    case 57:
    case 58:
    case 59:
        break;
    case 60: // ideogram underline or right side line
    case 61: // ideogram double underline or double line on the right side
    case 62: // ideogram overline or left side line
    case 63: // ideogram double overline or double line on the left side
    case 64: // ideogram stress marking
    case 65: // cancels the effect of the rendition aspects established by parameter values 60 to 64
        break;
    }
}

void SerialConnECMA48::ProcessSGR(const std::string& seq)
{
    // ECMA48 SGR, [Ps...]<m>
    std::string ps = seq.substr(1, seq.size()-2);
    if (ps.empty()) {
        ProcessSGR(0);
    } else {
        SplitString(std::move(ps), ';', [=](const char* token) {
            ProcessSGR(atoi(token));
        });
    }
}

void SerialConnECMA48::ProcessCBT(const std::string& seq)
{
    int cellsz = mVx - (mVx / 8 * 8);
    std::string token = seq.substr(1, seq.length() - 2);
    int p = 1;
    if (!token.empty()) {
        p = atoi(token.c_str());
    }
    if (cellsz != 0) p--;
    while (p--) {
        cellsz += 8;
    }
    mVx = mVx - cellsz;
    if (mVx < 0) mVx = 0;
}

void SerialConnECMA48::ProcessCHA(const std::string& seq)
{
    ASSERT(seq.size() > 2);
    // [ Pn 0x47
    int xp = atoi(seq.substr(1, seq.size()-2).c_str());
    if (mVy >= 0 && mVy < mLines.size()) {
        if (xp >= 0 && xp < (int)mLines[mVy].size()) {
            mVx = xp;
        }
    }
}
void SerialConnECMA48::ProcessCHT(const std::string& seq)
{
    ASSERT(seq.size() > 2);
    // [ Pn 0x49
    int cellsz = mVx - (mVx / 8 * 8);
    std::string token = seq.substr(1, seq.length() - 2);
    int p = 1;
    if (!token.empty()) {
        p = atoi(token.c_str());
    }
    if (cellsz != 0) p--;
    while (p--) {
        cellsz += 8;
    }
    mVx = mVx + cellsz;
    if (mVx >= (int)mLines[mVy].size()) {
        mVx = (int)mLines[mVy].size() - 1;
    }
}

void SerialConnECMA48::ProcessCNL(const std::string& seq)
{
    ASSERT(seq.size() > 2);
    // [ Pn 0x49
    int p1 = atoi(seq.substr(1, seq.size()-2).c_str());
    int yn = mVy + p1;
    if (yn >= 0 && yn < (int)mLines.size()) {
        mVx = 0;
        mVy = p1;
    }
}

void SerialConnECMA48::ProcessCPL(const std::string& seq)
{
    ASSERT(seq.size() > 2);
    // [ Pn 0x49
    int p1 = atoi(seq.substr(1, seq.size()-2).c_str());
    int yn = mVy - p1;
    if (yn >= 0 && yn < (int)mLines.size()) {
        mVx = 0;
        mVy = p1;
    }
}

void SerialConnECMA48::ProcessCPR(const std::string& seq)
{
    // [ P1;P2 0x52, used to report position
}

void SerialConnECMA48::ProcessCUB(const std::string& seq)
{
    // cursor left, [ Pn 04/04 ]
    std::string token = seq.substr(1, seq.length() - 2);
    int p = 1;
    if (!token.empty()) {
        p = atoi(token.c_str());
    }
    mVx = std::max(0, mVx - p);
}
//
void SerialConnECMA48::ProcessCUD(const std::string& seq)
{
    // down
    std::string token = seq.substr(1, seq.length() - 2);
    int p = 1;
    if (!token.empty()) {
        p = atoi(token.c_str());
    }
    mVy += p; // allow the user move vy out of range.
}
//
void SerialConnECMA48::ProcessCUU(const std::string& seq)
{
    // up
    std::string token = seq.substr(1, seq.length() - 2);
    int p = 1;
    if (!token.empty()) {
        p = atoi(token.c_str());
    }
    mVy = std::max(0, mVy - p);
}
//
void SerialConnECMA48::ProcessCUF(const std::string& seq)
{
    Size csz = GetConsoleSize();
    // right
    std::string token = seq.substr(1, seq.length() - 2);
    int p = 1;
    if (!token.empty()) {
        p = atoi(token.c_str());
    }
    mVx += p;  // allow the user move vx out of range.
}
//
void SerialConnECMA48::ProcessCUP(const std::string& seq)
{
    Size csz = GetConsoleSize();
    // set position, [Pn1;Pn2          - row;column
    int p[2] = {0}, idx = 0;
    SplitString(seq.substr(1, seq.length()-2), ';', [&](const char* token) {
        if (idx < 2 && token[0] != '\0') {
            p[idx++] = atoi(token) - 1;
        }
    });
    // limit to data-component range, why ?
    // Because the sender will send a [999999;99999H seq, how do you process it?
    // I guess it's meaning is "Please move the cursor to end of data".
    if (p[0] < 0) p[0] = 0; else if (p[0] >= csz.cy) p[0] = csz.cy-1;
    if (p[1] < 0) p[1] = 0; else if (p[1] >= csz.cx) p[1] = csz.cx-1;
    mVx = p[1];
    mVy = p[0];
}

void SerialConnECMA48::ProcessCTC(int ps)
{
    // mLines is the result
    switch (ps) {
    case 0: if (1) { // set tab
        ProcessC0(0x09);
    } break;
    case 1: if (1) { // line tab
        ProcessC0(0x0b);
    } break;
    case 2: if (1) { // clear tab
        if (mVy >= 0 && mVy < (int)mLines.size() &&
            mVx >= 0 && mVx < (int)mLines[mVy].size())
        {
            // we do not consider TSM mode, only support Single line
            if (mLines[mVy][mVx].Code() == '\t') {
                mLines[mVy].erase(mLines[mVy].begin() + mVx);
                mVx--;
            }
        }
    } break;
    case 3: if (1) { // clear line tab
        if (mVy >= 0 && mVy < (int)mLines.size() &&
            mVx >= 0 && mVx < (int)mLines[mVy].size())
        {
            // merge
            if (mLines[mVy][mVx].Code() == '\v') {
                mLines[mVy][mVx].SetCode(' '); // The \v is the last char of current line,
                                                  // we replace it with a blank char,
                                                  // if the next line exists, then merge it
                                                  // into current line, that op will override
                                                  // this blank char.
                VTLine vline;
                vline.insert(mLines[mVy].begin(), mLines[mVy].begin() + mVx, vline.begin());
                if (mVy < (int)mLines.size() - 1) {
                    for (int i = mVx; i < (int)mLines[mVy+1].size(); ++i) {
                        vline.push_back(mLines[mVy+1][i]);
                    }
                    mLines[mVy] = std::move(vline);
                    mLines.erase(mLines.begin() + mVy+1);
                    Size csz = GetConsoleSize();
                        mLines.push_back(VTLine(csz.cx, ' ').SetHeight(mFontH));
                }
            }
        }
    } break;
    case 4: if (1) { // clear all tabs in active line
                     // we do not consider TSM mode, only support Single line
        if (mVy >= 0 && mVy < (int)mLines.size() &&
            mVx >= 0 && mVx < (int)mLines[mVy].size())
        {
            VTLine vline;
            for (size_t k = 0; k < mLines[mVy].size(); ++k) {
                if (mLines[mVy][k].Code() != '\t') {
                    vline.push_back(mLines[mVy][k]);
                }
            }
            for (size_t k = vline.size(); k < mLines[mVy].size(); ++k) {
                vline.push_back(' ');
            }
            mLines[mVy] = std::move(vline);
        }
    } break;
    case 5: if (1) { // clear all tabs
        for (size_t i = 0; i < mLines.size(); ++i) {
            VTLine vline;
            for (size_t j = 0; j < mLines[i].size(); ++j) {
                if (mLines[i][j].Code() != '\t') {
                    vline.push_back(mLines[i][j]);
                }
            }
            for (size_t k = vline.size(); k < mLines[i].size(); ++k) {
                vline.push_back(' ');
            }
            mLines[i] = std::move(vline);
        }
        break;
    } break;
    case 6: if (1) { // clear all line tabs
        std::vector<VTLine> lines;
        for (size_t i = 0; i < mLines.size(); ++i) {
            VTLine vline;
            for (size_t j = 0; j < mLines[i].size(); ++j) {
                if (mLines[i][j].Code() == '\v') {
                    continue;
                }
                vline.push_back(mLines[i][j]);
            }
            lines.push_back(std::move(vline));
        }
        Size csz = GetConsoleSize();
        for (size_t k = lines.size(); k < mLines.size(); ++k) {
            lines.push_back(VTLine(csz.cx, ' ').SetHeight(mFontH));
        }
        mLines = std::move(lines);
    } break;
    }
}

void SerialConnECMA48::ProcessCTC(const std::string& seq)
{
    std::string ps = seq.substr(1, seq.size()-2);
    if (!ps.empty()) {
        ProcessCTC(0);
    } else {
        SplitString(std::move(ps), ';',
            [=](const char* token) { ProcessCTC(atoi(token)); });
    }
}

void SerialConnECMA48::ProcessCVT(const std::string& seq)
{
    std::string token = seq.substr(1, seq.length() - 2);
    int p = 1;
    if (!token.empty()) {
        p = atoi(token.c_str());
    }
    mVy = std::min(mVy + p, (int)mLines[mVy].size()-1);
}

void SerialConnECMA48::ProcessDA(const std::string& seq)
{
    // ECMA48 is protocol, it tells us how to request/response.
}

void SerialConnECMA48::ProcessDAQ(const std::string& seq)
{
}

void SerialConnECMA48::ProcessDCH(const std::string& seq)
{
    int pn = 1;
    std::string pn_s = seq.substr(1, seq.length()-2);
    if (!pn_s.empty()) {
        pn = atoi(pn_s.c_str());
    }
    // Delete the char, not erasing, so we should pad the line.
    if (mModes.HEM == Ecma48Modes::HemFollowing) {
        int p_ = mVx + pn;
        if (p_ >= (int)mLines[mVy].size()) {
            p_ = (int)mLines[mVy].size()-1;
        }
        mLines[mVy].erase(mLines[mVy].begin() + mVx, mLines[mVy].begin() + p_);
    } else { // Preceding
        int p_ = mVx - pn;
        if (p_ < 0) p_ = 0;
        mLines[mVy].erase(mLines[mVy].begin() + p_, mLines[mVy].begin() + mVx);
    }
    // Why we do not pad the current line ?
    // Actually, we do not need to do this. After every control function, or C0,C1
    // the VT(base class) will invoke ProcessOverflowlines which will pad the lines.
}

void SerialConnECMA48::ProcessDL(const std::string& seq)
{
    int pn = 1;
    std::string pn_s = seq.substr(1, seq.length()-2);
    if (!pn_s.empty()) {
        pn = atoi(pn_s.c_str());
    }
    Size csz = GetConsoleSize();
    VTLine vline(csz.cx, mBlankChar); vline.SetHeight(mFontH);
    if (mModes.VEM == Ecma48Modes::VemFollowing) {
        int p_ = mVy + pn;
        if (p_ >= (int)mLines.size()) {
            p_ = (int)mLines.size() - 1;
        }
        int bot = mScrollingRegion.Bottom; if (bot < 0) bot = csz.cy - 1;
        mLines.erase(mLines.begin() + mVy, mLines.begin() + p_);
        bot -= pn; if (bot < 0) bot = 0;
        mLines.insert(mLines.begin() + bot, pn, vline);
    } else {
        int p_ = mVy - pn;
        if (p_ < 0)
            p_ = 0;
        int top = mScrollingRegion.Top;
        mLines.erase(mLines.begin() + mVy, mLines.begin() + p_);
        mLines.insert(mLines.begin() + top, pn, vline);
    }
}

void SerialConnECMA48::ProcessDSR(const std::string& seq)
{
    // [ ps 0x6e
    int ps = atoi(seq.substr(1, seq.length()-2).c_str());
    switch (ps) {
    case 0: // ready, no malfunction detected
        break;
    case 1: // busy, another DSR must be requested later
        break;
    case 2: // busy, another DSR will be sent later
        break;
    case 3: // some malfunction detected, another DSR must be requested later
        break;
    case 4: // some malfunction detected, another DSR will be sent later
        break;
    case 5: // a DSR is requested
        break;
    case 6:
        // a report of the active presentation position or of the active data position in the form of
        // ACTIVE POSITION REPORT (CPR) is requested.
        // The presentation position is the logical position, in pixels. We return data
        // position.
        std::string rsp = "\033["
                          + std::to_string(mVy+1) + ";"
                          + std::to_string(mVx+1) + "R";
        GetSerial()->Write(rsp);
        break;
    }
}

void SerialConnECMA48::ProcessDTA(const std::string& seq)
{
    // [ pn1;pn2 0x20 0x54
    // define a subsequent page
    int p[2] = {0}, idx = 0;
    SplitString(seq.substr(1, seq.length()-3), ';', [&](const char* token) {
        if (idx < 2) {
            p[idx++] = atoi(token);
        }
    });
    if (p[0] <= 0) p[0] = 1;
    // update subsequent page span.
    mScrollingRegion.Top = p[0]-1;
    mScrollingRegion.Bottom = p[1]-1;
    // check
    CheckAndFix(mScrollingRegion);
}

void SerialConnECMA48::ProcessEA(const std::string& seq)
{
}

void SerialConnECMA48::ProcessECH(const std::string& seq)
{
    int pn = 1;
    std::string pn_s = seq.substr(1, seq.length()-2);
    if (!pn_s.empty()) {
        pn = atoi(pn_s.c_str());
    }
    for (int i = mVx; i < pn && i < (int)mLines[mVy].size(); ++i) {
        mLines[mVy][i] = mBlankChar;
    }
}
// What's page? What's field?
// I ignore field, treat page as current virtual screen
void SerialConnECMA48::ProcessED(const std::string& seq)
{
    Size csz = GetConsoleSize();
    // erase in page, [ Ps 0x4a
    int ps = atoi(seq.substr(1, seq.length()-2).c_str());
    switch (ps) {
    case 0: if (1) { // current position to end of the page
        for (int i = mVx; i < (int)mLines[mVy].size(); ++i) {
            mLines[mVy][i] = mBlankChar;
        }
        for (int i = mVy+1; i < (int)mLines.size(); ++i) {
            mLines[i] = VTLine(csz.cx, mBlankChar).SetHeight(mFontH);
        }
    } break;
    case 1: if (1) { // beginning of the page to current position
        mLines.erase(mLines.begin(), mLines.begin() + mVy);
        for (int i = 0; i < mVy-1; ++i) {
            mLines[i] = VTLine(csz.cx, mBlankChar).SetHeight(mFontH);
        }
    } break;
    case 2: if (1) { // all
        for (size_t k = 0; k < mLines.size(); ++k) {
            mLines[k] = VTLine(csz.cx, mBlankChar).SetHeight(mFontH);
        }
    } break;
    }
}

void SerialConnECMA48::ProcessEF(const std::string& seq)
{
    // erase in field, [ Ps 0x4e
    int ps = atoi(seq.substr(1, seq.length()-2).c_str());
    switch (ps) {
    case 0: if (1) {
    } break;
    }
}

void SerialConnECMA48::ProcessEL(const std::string& seq)
{
    // erase in line, [ Ps 0x4b
    int ps = atoi(seq.substr(1, seq.length()-2).c_str());
    switch (ps) {
    case 0: if (1) { // current pos to end of line
        for (int i = mVx; i < (int)mLines[mVy].size(); ++i) {
            mLines[mVy][i] = mBlankChar;
        }
    } break;
    case 1: if (1) { // beginning of line to current pos
        for (int i = 0; i <= mVx; ++i) {
            mLines[mVy][i] = mBlankChar;
        }
    } break;
    case 2: if (1) { // erase line
        Size csz = GetConsoleSize();
        mLines[mVy] = VTLine(csz.cx, mBlankChar).SetHeight(mFontH);
    } break;
    }
}

void SerialConnECMA48::ProcessFNK(const std::string& seq)
{
}
void SerialConnECMA48::ProcessFNT(const std::string& seq)
{
}
void SerialConnECMA48::ProcessGCC(const std::string& seq)
{
}
void SerialConnECMA48::ProcessGSM(const std::string& seq)
{
}
void SerialConnECMA48::ProcessGSS(const std::string& seq)
{
}

void SerialConnECMA48::ProcessHPA(const std::string& seq)
{
    int pn = 1;
    std::string pn_s = seq.substr(1, seq.length()-2);
    if (!pn_s.empty()) {
        pn = atoi(pn_s.c_str());
    }
    pn -= 1;
    if (pn < 0) pn = 0;
    else if (pn >= (int)mLines[mVy].size())
        pn = (int)mLines[mVy].size()-1;
    mVx = pn;
}

void SerialConnECMA48::ProcessHPB(const std::string& seq)
{
    int pn = 1;
    std::string pn_s = seq.substr(1, seq.length()-2);
    if (!pn_s.empty()) {
        pn = atoi(pn_s.c_str());
    }
    int p = mVx - pn;
    if (p < 0) p = 0;
    mVx = p;
}

void SerialConnECMA48::ProcessHPR(const std::string& seq)
{
    int pn = 1;
    std::string pn_s = seq.substr(1, seq.length()-2);
    if (!pn_s.empty()) {
        pn = atoi(pn_s.c_str());
    }
    int p = mVx + pn;
    if (p >= (int)mLines[mVy].size()) p = (int)mLines[mVy].size()-1;
    mVx = p;
}

void SerialConnECMA48::ProcessHVP(const std::string& seq)
{
    // [ pn1;pn2 0x66
    int p[2] = {1,1}, idx = 0;
    SplitString(seq.substr(1, seq.length()-2), ';', [&](const char* token) {
        if (idx < 2) {
            p[idx++] = atoi(token);
        }
    });
    // check and fix
    Size csz = GetConsoleSize();
    p[0] -= 1; p[1] -= 1;
    if (p[0] < 0) p[0] = 0; else if (p[0] >= csz.cy) p[0] = csz.cy-1;
    if (p[1] < 0) p[1] = 0; else if (p[1] >= csz.cx) p[1] = csz.cx-1;
    mVx = p[1]; mVy = p[0];
}

void SerialConnECMA48::ProcessICH(const std::string& seq)
{
    // insert character, [ pn 0x40
    int pn = 1;
    std::string pn_s = seq.substr(1, seq.length()-2);
    if (!pn_s.empty()) {
        pn = atoi(pn_s.c_str());
    }
    if (pn < 1) return;
    // erase, not remove.
    if (mModes.HEM == Ecma48Modes::HemFollowing) {
        int cnt = 0;
        for (int i = mVx; i < (int)mLines[mVy].size(); ++i) {
            //mLines[mVy][i] = mBlankChar;
            mLines[mVy].insert(mLines[mVy].begin() + i, mBlankChar);
            if (++cnt >= pn) break;
        }
    } else {
        int cnt = 0;
        for (int i = mVx; i >= 0; --i) {
            //mLines[mVy][i] = mBlankChar;
            mLines[mVy].insert(mLines[mVy].begin() + i, mBlankChar);
            if (++cnt >= pn) break;
        }
    }
}

void SerialConnECMA48::ProcessIDCS(const std::string& seq)
{
}

void SerialConnECMA48::ProcessIGS(const std::string& seq)
{
}
//
void SerialConnECMA48::ProcessIL(const std::string& seq)
{
    int pn = 1;
    std::string pn_s = seq.substr(1, seq.length()-2);
    if (!pn_s.empty()) {
        pn = atoi(pn_s.c_str());
    }
    if (pn < 1) return;
    Size csz = GetConsoleSize();
    VTLine vline(csz.cx, mBlankChar); vline.SetHeight(mFontH);
    mLines.insert(mLines.begin() + mVy, pn, vline);
    if (mModes.HEM == Ecma48Modes::HemFollowing) {
        int bot = mScrollingRegion.Bottom;
        if (bot < 0) bot = csz.cy -1; bot += 1;
        mLines.erase(mLines.begin() + bot, mLines.begin() + bot + pn);
    } else {
        int top = mScrollingRegion.Top;
        mLines.erase(mLines.begin() + top, mLines.begin() + top + pn);
    }
    mVx = 0;
}

void SerialConnECMA48::ProcessJFY(const std::string& seq)
{
}

void SerialConnECMA48::ProcessMC(const std::string& seq)
{
}
void SerialConnECMA48::ProcessNP(const std::string& seq)
{
}
void SerialConnECMA48::ProcessPEC(const std::string& seq)
{
}
void SerialConnECMA48::ProcessPFS(const std::string& seq)
{
}
void SerialConnECMA48::ProcessPP(const std::string& seq)
{
}
void SerialConnECMA48::ProcessPPA(const std::string& seq)
{
}
void SerialConnECMA48::ProcessPPB(const std::string& seq)
{
}

void SerialConnECMA48::ProcessPTX(const std::string& seq)
{
}

void SerialConnECMA48::ProcessQUAD(const std::string& seq)
{
}
//
void SerialConnECMA48::ProcessREP(const std::string& seq)
{
    int pn = 1;
    std::string pn_s = seq.substr(1, seq.length()-2);
    if (!pn_s.empty()) {
        pn = atoi(pn_s.c_str());
    }
    if (pn < 1) return;
    // repeat preceding char N times
    int cn = 0;
    for (int k = 1; k <= pn && k < (int)mLines[mVy].size(); ++k) {
        mLines[mVy][mVx+k] = mLines[mVy][mVx];
        cn++;
    }
    for (int k = 0; k < pn - cn; ++k) {
        mLines[mVy].push_back(mLines[mVy][mVx]);
    }
    mVx += pn;
}
//
void SerialConnECMA48::ProcessRM(const std::string& seq)
{
	// CSI Ps... 6/12
	SplitString(seq.substr(1, seq.length()-1), ';', [=](const char* token) {
		int ps = atoi(token);
		switch (ps) {
		case 1: // GATM
			mModes.GATM = ~mModes.GATM;
			break;
		case 2: // KAM
			mModes.KAM = ~mModes.KAM;
			break;
		case 3:
			mModes.CRM = ~mModes.CRM;
			break;
		case 4:
			mModes.IRM = ~mModes.IRM;
			break;
		case 5:
			mModes.SRTM = ~mModes.SRTM;
			break;
		case 6:
			mModes.ERM = ~mModes.ERM;
			break;
		case 7:
			mModes.VEM = ~mModes.VEM;
			break;
		case 8:
			mModes.BDSM = ~mModes.BDSM;
			break;
		case 9:
			mModes.DCSM = ~mModes.DCSM;
			break;
		case 10:
			mModes.HEM = ~mModes.HEM;
			break;
		case 11: // deprecated
			break;
		case 12:
			mModes.SRM = ~mModes.SRM;
			break;
		case 13:
			mModes.FEAM = ~mModes.FEAM;
			break;
		case 14:
			mModes.FETM = ~mModes.FETM;
			break;
		case 15:
			mModes.MATM = ~mModes.MATM;
			break;
		case 16:
			mModes.TIM = ~mModes.TIM;
			break;
		case 17:
			mModes.SATM = ~mModes.SATM;
			break;
		case 18:
			mModes.TSM = ~mModes.TSM;
			break;
		case 19:
		case 20:
			break;
		case 21:
			mModes.GRCM = ~mModes.GRCM;
			break;
		case 22: // deprecated
			break;
		default:break;
		}
	});
}
//
void SerialConnECMA48::ProcessSACS(const std::string& seq)
{
}
//
void SerialConnECMA48::ProcessSAPV(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSCO(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSCP(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSCS(const std::string& seq)
{
}

void SerialConnECMA48::ProcessSD(const std::string& seq)
{
    int pn = 1;
    std::string pn_s = seq.substr(1, seq.length()-2);
    if (!pn_s.empty()) {
        pn = atoi(pn_s.c_str());
    }
    if (pn < 1) return;
    Size csz = GetConsoleSize();
    mLines.insert(mLines.begin(), pn, VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
    for (int i = 0; i < pn; ++i) {
        mLines.pop_back();
    }
}

void SerialConnECMA48::ProcessSDS(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSEE(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSEF(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSHS(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSIMD(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSL(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSLH(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSLL(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSLS(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSM(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSPD(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSPH(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSPI(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSPL(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSPQR(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSR(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSRCS(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSRS(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSSU(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSSW(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSTAB(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSU(const std::string& seq)
{
    int pn = 1;
    std::string pn_s = seq.substr(1, seq.length()-2);
    if (!pn_s.empty()) {
        pn = atoi(pn_s.c_str());
    }
    if (pn < 1) return;
    mVy += pn;
}
void SerialConnECMA48::ProcessSVS(const std::string& seq)
{
}
void SerialConnECMA48::ProcessTAC(const std::string& seq)
{
}
void SerialConnECMA48::ProcessTALE(const std::string& seq)
{
}
void SerialConnECMA48::ProcessTATE(const std::string& seq)
{
}
void SerialConnECMA48::ProcessTBC(const std::string& seq)
{
}
void SerialConnECMA48::ProcessTCC(const std::string& seq)
{
}
void SerialConnECMA48::ProcessTSR(const std::string& seq)
{
}
void SerialConnECMA48::ProcessTSS(const std::string& seq)
{
}
void SerialConnECMA48::ProcessVPA(const std::string& seq)
{
}
void SerialConnECMA48::ProcessVPB(const std::string& seq)
{
}

void SerialConnECMA48::ProcessVPR(const std::string& seq)
{
}

void SerialConnECMA48::InstallEcma48Functions()
{
    mEcma48Funcs[ECMA48_CBT ] = [=](const std::string& seq) { ProcessCBT(seq); };
    mEcma48Funcs[ECMA48_CHA ] = [=](const std::string& seq) { ProcessCHA(seq); };
    mEcma48Funcs[ECMA48_CHT ] = [=](const std::string& seq) { ProcessCHT(seq); };
    mEcma48Funcs[ECMA48_SGR ] = [=](const std::string& seq) { ProcessSGR(seq); };
    mEcma48Funcs[ECMA48_C1  ] = [=](const std::string& seq) { ProcessC1(seq[0]); };
    mEcma48Funcs[ECMA48_CTC ] = [=](const std::string& seq) { ProcessCTC(seq); };
    mEcma48Funcs[ECMA48_CUB ] = [=](const std::string& seq) { ProcessCUB(seq); };
    mEcma48Funcs[ECMA48_CUD ] = [=](const std::string& seq) { ProcessCUD(seq); };
    mEcma48Funcs[ECMA48_CUF ] = [=](const std::string& seq) { ProcessCUF(seq); };
    mEcma48Funcs[ECMA48_CUP ] = [=](const std::string& seq) { ProcessCUP(seq); };
    mEcma48Funcs[ECMA48_CUU ] = [=](const std::string& seq) { ProcessCUU(seq); };
    mEcma48Funcs[ECMA48_CVT ] = [=](const std::string& seq) { ProcessCVT(seq); };
    mEcma48Funcs[ECMA48_DA  ] = [=](const std::string& seq) { ProcessDA(seq); };
    mEcma48Funcs[ECMA48_DAQ ] = [=](const std::string& seq) { ProcessDAQ(seq); };
    mEcma48Funcs[ECMA48_DCH ] = [=](const std::string& seq) { ProcessDCH(seq); };
    mEcma48Funcs[ECMA48_DL  ] = [=](const std::string& seq) { ProcessDL(seq); };
    mEcma48Funcs[ECMA48_DSR ] = [=](const std::string& seq) { ProcessDSR(seq); };
    mEcma48Funcs[ECMA48_DTA ] = [=](const std::string& seq) { ProcessDTA(seq); };
    mEcma48Funcs[ECMA48_EA  ] = [=](const std::string& seq) { ProcessEA(seq); };
    mEcma48Funcs[ECMA48_ECH ] = [=](const std::string& seq) { ProcessECH(seq); };
    mEcma48Funcs[ECMA48_ED  ] = [=](const std::string& seq) { ProcessED(seq); };
    mEcma48Funcs[ECMA48_EF  ] = [=](const std::string& seq) { ProcessEF(seq); };
    mEcma48Funcs[ECMA48_EL  ] = [=](const std::string& seq) { ProcessEL(seq); };
    mEcma48Funcs[ECMA48_FNK ] = [=](const std::string& seq) { ProcessFNK(seq); };
    mEcma48Funcs[ECMA48_FNT ] = [=](const std::string& seq) { ProcessFNT(seq); };
    mEcma48Funcs[ECMA48_GCC ] = [=](const std::string& seq) { ProcessGCC(seq); };
    mEcma48Funcs[ECMA48_GSM ] = [=](const std::string& seq) { ProcessGSM(seq); };
    mEcma48Funcs[ECMA48_GSS ] = [=](const std::string& seq) { ProcessGSS(seq); };
    mEcma48Funcs[ECMA48_HPA ] = [=](const std::string& seq) { ProcessHPA(seq); };
    mEcma48Funcs[ECMA48_HPB ] = [=](const std::string& seq) { ProcessHPB(seq); };
    mEcma48Funcs[ECMA48_HPR ] = [=](const std::string& seq) { ProcessHPR(seq); };
    mEcma48Funcs[ECMA48_HVP ] = [=](const std::string& seq) { ProcessHVP(seq); };
    mEcma48Funcs[ECMA48_ICH ] = [=](const std::string& seq) { ProcessICH(seq); };
    mEcma48Funcs[ECMA48_IDCS] = [=](const std::string& seq) { ProcessIDCS(seq); };
    mEcma48Funcs[ECMA48_IGS ] = [=](const std::string& seq) { ProcessIGS(seq); };
    mEcma48Funcs[ECMA48_IL  ] = [=](const std::string& seq) { ProcessIL(seq); };
    mEcma48Funcs[ECMA48_JFY ] = [=](const std::string& seq) { ProcessJFY(seq); };
    mEcma48Funcs[ECMA48_MC  ] = [=](const std::string& seq) { ProcessMC(seq); };
    mEcma48Funcs[ECMA48_NP  ] = [=](const std::string& seq) { ProcessNP(seq); };
    mEcma48Funcs[ECMA48_PEC ] = [=](const std::string& seq) { ProcessPEC(seq); };
    mEcma48Funcs[ECMA48_PFS ] = [=](const std::string& seq) { ProcessPFS(seq); };
    mEcma48Funcs[ECMA48_PP  ] = [=](const std::string& seq) { ProcessPP(seq); };
    mEcma48Funcs[ECMA48_PPA ] = [=](const std::string& seq) { ProcessPPA(seq); };
    mEcma48Funcs[ECMA48_PPB ] = [=](const std::string& seq) { ProcessPPB(seq); };
    mEcma48Funcs[ECMA48_PTX ] = [=](const std::string& seq) { ProcessPTX(seq); };
    mEcma48Funcs[ECMA48_QUAD] = [=](const std::string& seq) { ProcessQUAD(seq); };
    mEcma48Funcs[ECMA48_REP ] = [=](const std::string& seq) { ProcessREP(seq); };
    mEcma48Funcs[ECMA48_RM  ] = [=](const std::string& seq) { ProcessRM(seq); };
    mEcma48Funcs[ECMA48_SACS] = [=](const std::string& seq) { ProcessSACS(seq); };
    mEcma48Funcs[ECMA48_SAPV] = [=](const std::string& seq) { ProcessSAPV(seq); };
    mEcma48Funcs[ECMA48_SCO ] = [=](const std::string& seq) { ProcessSCO(seq); };
    mEcma48Funcs[ECMA48_SCP ] = [=](const std::string& seq) { ProcessSCP(seq); };
    mEcma48Funcs[ECMA48_SCS ] = [=](const std::string& seq) { ProcessSCS(seq); };
    mEcma48Funcs[ECMA48_SD  ] = [=](const std::string& seq) { ProcessSD(seq); };
    mEcma48Funcs[ECMA48_SDS ] = [=](const std::string& seq) { ProcessSDS(seq); };
    mEcma48Funcs[ECMA48_SEE ] = [=](const std::string& seq) { ProcessSEE(seq); };
    mEcma48Funcs[ECMA48_SEF ] = [=](const std::string& seq) { ProcessSEF(seq); };
    mEcma48Funcs[ECMA48_SHS ] = [=](const std::string& seq) { ProcessSHS(seq); };
    mEcma48Funcs[ECMA48_SIMD] = [=](const std::string& seq) { ProcessSIMD(seq); };
    mEcma48Funcs[ECMA48_SL  ] = [=](const std::string& seq) { ProcessSL(seq); };
    mEcma48Funcs[ECMA48_SLH ] = [=](const std::string& seq) { ProcessSLH(seq); };
    mEcma48Funcs[ECMA48_SLL ] = [=](const std::string& seq) { ProcessSLL(seq); };
    mEcma48Funcs[ECMA48_SLS ] = [=](const std::string& seq) { ProcessSLS(seq); };
    mEcma48Funcs[ECMA48_SM  ] = [=](const std::string& seq) { ProcessSM(seq); };
    mEcma48Funcs[ECMA48_SPD ] = [=](const std::string& seq) { ProcessSPD(seq); };
    mEcma48Funcs[ECMA48_SPH ] = [=](const std::string& seq) { ProcessSPH(seq); };
    mEcma48Funcs[ECMA48_SPI ] = [=](const std::string& seq) { ProcessSPI(seq); };
    mEcma48Funcs[ECMA48_SPL ] = [=](const std::string& seq) { ProcessSPL(seq); };
    mEcma48Funcs[ECMA48_SPQR] = [=](const std::string& seq) { ProcessSPQR(seq); };
    mEcma48Funcs[ECMA48_SR  ] = [=](const std::string& seq) { ProcessSR(seq); };
    mEcma48Funcs[ECMA48_SRCS] = [=](const std::string& seq) { ProcessSRCS(seq); };
    mEcma48Funcs[ECMA48_SRS ] = [=](const std::string& seq) { ProcessSRS(seq); };
    mEcma48Funcs[ECMA48_SSU ] = [=](const std::string& seq) { ProcessSSU(seq); };
    mEcma48Funcs[ECMA48_SSW ] = [=](const std::string& seq) { ProcessSSW(seq); };
    mEcma48Funcs[ECMA48_STAB] = [=](const std::string& seq) { ProcessSTAB(seq); };
    mEcma48Funcs[ECMA48_SU  ] = [=](const std::string& seq) { ProcessSU(seq); };
    mEcma48Funcs[ECMA48_SVS ] = [=](const std::string& seq) { ProcessSVS(seq); };
    mEcma48Funcs[ECMA48_TAC ] = [=](const std::string& seq) { ProcessTAC(seq); };
    mEcma48Funcs[ECMA48_TALE] = [=](const std::string& seq) { ProcessTALE(seq); };
    mEcma48Funcs[ECMA48_TATE] = [=](const std::string& seq) { ProcessTATE(seq); };
    mEcma48Funcs[ECMA48_TBC ] = [=](const std::string& seq) { ProcessTBC(seq); };
    mEcma48Funcs[ECMA48_TCC ] = [=](const std::string& seq) { ProcessTCC(seq); };
    mEcma48Funcs[ECMA48_TSR ] = [=](const std::string& seq) { ProcessTSR(seq); };
    mEcma48Funcs[ECMA48_TSS ] = [=](const std::string& seq) { ProcessTSS(seq); };
    mEcma48Funcs[ECMA48_VPA ] = [=](const std::string& seq) { ProcessVPA(seq); };
    mEcma48Funcs[ECMA48_VPB ] = [=](const std::string& seq) { ProcessVPB(seq); };
    mEcma48Funcs[ECMA48_VPR ] = [=](const std::string& seq) { ProcessVPR(seq); };
}

void SerialConnECMA48::ProcessEcma48Trivial(const std::string& seq)
{
    auto it = mEcma48TrivialHandlers.find(seq);
    if (it != mEcma48TrivialHandlers.end()) {
        it->second();
    }
}

int SerialConnECMA48::IsControlSeq(const std::string& seq)
{
    int ret = IsECMA48ControlSeq(seq);
    if (ret == 0) {
        ret = Superclass::IsControlSeq(seq);
    }
    return ret;
}

bool SerialConnECMA48::ProcessControlSeq(const std::string& seq, int seq_type)
{
    // for faster, we forward the seq_type as accurately as we could
    if (seq_type == ECMA48_Trivial) ProcessEcma48Trivial(seq); else {
        if (seq_type > ECMA48_Trivial && seq_type < ECMA48_SeqType_Endup) {
            auto it = mEcma48Funcs.find(seq_type);
            if (it != mEcma48Funcs.end()) {
                it->second(seq);
            }
        } else {
            return Superclass::ProcessControlSeq(seq, seq_type);
        }
    }
    return true;
}

bool SerialConnECMA48::ProcessC0(char cc)
{
    // process those needed, the super class will process others
    switch (cc) {
    case 0x00: break; // NUL
    case 0x03: break; // ETX
    case 0x04: break; // EOT
    case 0x05: break; // ENQ
    case 0x07: break; // BEL
    case 0x0c: // FF
        mVy += 1;
        break;
    // WE DO NOT NEED THESE, because we support them in other ways.
    case 0x0e: break; // SO
    case 0x0f: break; // SI
    case 0x11: break; // DC1
    case 0x13: break; // DC3
    case 0x18: // CAN
    case 0x1a: // SUB, processed as CAN
        break;
    default:return false;
    }
    return true;
}

bool SerialConnECMA48::ProcessC1(char cc)
{
	switch (cc) {
	case 0x4d: if (1) { // ESC M, RI
		int top = mScrollingRegion.Top, bot = mScrollingRegion.Bottom;
		if (bot < 0) bot = (int)mLines.size() - 1;
		if (mVy > top) mVy--; else {
			Size csz = GetConsoleSize();
			int dy = mVy - top + 1;
			mLines.insert(mLines.begin() + top, dy, VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
			mLines.erase(mLines.begin() + bot+1, mLines.begin() + bot+1+dy);
			mVy = top;
		}
	} break;
	}
    return true;
}

bool SerialConnECMA48::ProcessAsciiControlChar(char cc)
{
    bool processed = ProcessC0(cc);
    if (!processed) {
        processed = Superclass::ProcessAsciiControlChar(cc);
    }
    return processed;
}

bool SerialConnECMA48::ProcessKeyDown(Upp::dword key, Upp::dword flags)
{
    bool processed = false;
    if (flags == 0) {
        std::string seq;
        switch (key) {
        case K_UP:    seq = "\x1b[\x41"; break;
        case K_DOWN:  seq = "\x1b[\x42"; break;
        case K_LEFT:  seq = "\x1b[\x44"; break;
        case K_RIGHT: seq = "\x1b[\x43"; break;
        default:break;
        }
        if (!seq.empty()) {
            processed = true;
            GetSerial()->Write(seq);
        }
    }
    if (!processed) {
        return Superclass::ProcessKeyDown(key, flags);
    }
    return processed;
}
