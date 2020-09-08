//
// (c) 2020 chiv
//
#include "SerialConnECMA48.h"
#include "ECMA48ControlSeq.h"
#include "ConnFactory.h"
// We implement this as the base-class of Xterm, we'll provide ecma48 in future.
REGISTER_CONN_INSTANCE("ECMA-48", SerialConnECMA48);

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
        mCurrAttrFuncs.clear();
        mCurrAttrFuncs.push_back([=]() { SetDefaultStyle(); });
        break;
    case 1:
        mCurrAttrFuncs.push_back([=]() {
            mFont.Bold();
        });
        break;
    case 2:
        mCurrAttrFuncs.push_back([=]() {
            mFont.NoBold();
        });
        break;
    case 3:
        mCurrAttrFuncs.push_back([=]() {
            mFont.Italic();
        });
        break;
    case 4:
        mCurrAttrFuncs.push_back([=]() {
            mFont.Underline();
        });
        break;
    case 5:
    case 6: // rapidly blinking, No, we treat is as 5
        mCurrAttrFuncs.push_back([=]() {
            mBlink = true;
        });
        break;
    case 7:
        mCurrAttrFuncs.push_back([=]() {
            std::swap(mFgColor, mBgColor);
        });
        break;
    case 8: // conceal
        mCurrAttrFuncs.push_back([=]() {
            mVisible = false;
        });
        break;
    case 9: // crossed-out
        mCurrAttrFuncs.push_back([=]() {
            mFont.Strikeout();
        });
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
        mCurrAttrFuncs.push_back([=]() {
            mFont.NoBold();
            mFgColor = mDefaultFgColor;
        });
        break;
    case 23: // Not italicized, not fraktur
        mCurrAttrFuncs.push_back([=]() {
            mFont.NoItalic();
        });
        break;
    case 24: // not underline
        mCurrAttrFuncs.push_back([=]() {
            mFont.NoUnderline();
        });
        break;
    case 25: // steady, no blinking
        mCurrAttrFuncs.push_back([=]() {
            mBlink = false;
        });
        break;
    case 26: // reserved for proportional spacing as specified in CCITT recommendation T6.1
    case 27: // positive image
        break;
    case 28: // revealed characters
        mCurrAttrFuncs.push_back([=]() {
            mVisible = true;
        });
        break;
    case 29: // not crossed
        mCurrAttrFuncs.push_back([=]() {
            mFont.NoStrikeout();
        });
        break;
    case 30: // black
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(0, 0, 0);
        });
        break;
    case 31: // red
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(255, 0, 0);
        });
        break;
    case 32: // green
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(0, 255, 0);
        });
        break;
    case 33: // yellow
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(255, 255, 0);
        });
        break;
    case 34: // blue
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(0, 0, 255);
        });
        break;
    case 35: // magenta
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(255, 0, 255);
        });
        break;
    case 36: // cyan
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(0, 255, 255);
        });
        break;
    case 37: // white
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = Color(255, 255, 255);
        });
        break;
    case 38: // (reserved for future standardization; intended for setting character foreground colour as specified in
             // ISO 8613-6 [CCITT Recommendation T.416])
        break;
    case 39: // default color
        mCurrAttrFuncs.push_back([=]() {
            mFgColor = mDefaultFgColor;
        });
        break;
    case 40: // black background
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(0, 0, 0);
        });
        break;
    case 41: // red bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(255, 0, 0);
        });
        break;
    case 42: // green bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(0, 255, 0);
        });
        break;
    case 43: // yellow bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(255, 255, 0);
        });
        break;
    case 44: // blue bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(0, 0, 255);
        });
        break;
    case 45: // magenta bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(255, 0, 255);
        });
        break;
    case 46: // cyan bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(0, 255, 255);
        });
        break;
    case 47: // white bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = Color(255, 255, 255);
        });
        break;
    case 48: // (reserved for future standardization; intended for setting character foreground colour as specified in
             // ISO 8613-6 [CCITT Recommendation T.416])
        break;
    case 49: // default color bg
        mCurrAttrFuncs.push_back([=]() {
            mBgColor = mDefaultBgColor;
        });
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
    SplitString(seq.substr(1, seq.size()-2), ';', [=](const char* token) {
        ProcessSGR(atoi(token));
    });
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
    int p1 = atoi(seq.substr(1, seq.size()-2).c_str());
    if (mVy >= 0 && mVy < mLines.size()) {
        mVx = mVx / 4 * 4 + p1*4;
        if (mVx >= (int)mLines[mVy].size()) {
            mVx = (int)mLines[mVy].size()-1;
        }
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
    mVx = std::min(csz.cx-1, mVy - p);
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
    if (p[0] < 0) p[0] = 0; else if (p[0] >= csz.cy) p[0] = csz.cy-1;
    if (p[1] < 0) p[1] = 0; else if (p[1] >= csz.cx) p[1] = csz.cx-1;
    mVx = p[1];
    mVy = p[0];
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
            if (mLines[mVy][mVx] == '\t') {
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
            if (mLines[mVy][mVx] == '\v') {
                mLines[mVy][mVx] = ' '; // The \v is the last char of current line,
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
                if (mLines[mVy][k] != '\t') {
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
                if (mLines[i][j] != '\t') {
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
                if (mLines[i][j] == '\v') {
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
    SplitString(seq.substr(1, seq.size()-2), ';',
        [=](const char* token) { ProcessCTC(atoi(token)); });
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
}
// Not support now.
void SerialConnECMA48::ProcessDAQ(const std::string& seq)
{
}

void SerialConnECMA48::ProcessDCH(const std::string& seq)
{
}

void SerialConnECMA48::ProcessDL(const std::string& seq)
{
}
void SerialConnECMA48::ProcessDSR(const std::string& seq)
{
}
void SerialConnECMA48::ProcessDTA(const std::string& seq)
{
}
void SerialConnECMA48::ProcessEA(const std::string& seq)
{
}
void SerialConnECMA48::ProcessECH(const std::string& seq)
{
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
        for (int i = 0; i < mVy-1; ++i) {
            mLines[i] = VTLine(csz.cx, mBlankChar).SetHeight(mFontH);
        }
        for (int i = 0; i <= mVx; ++i) {
            mLines[mVy][i] = mBlankChar;
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
}
void SerialConnECMA48::ProcessHPB(const std::string& seq)
{
}
void SerialConnECMA48::ProcessHPR(const std::string& seq)
{
}
void SerialConnECMA48::ProcessHVP(const std::string& seq)
{
}
void SerialConnECMA48::ProcessICH(const std::string& seq)
{
}
void SerialConnECMA48::ProcessIDCS(const std::string& seq)
{
}
void SerialConnECMA48::ProcessIGS(const std::string& seq)
{
}
void SerialConnECMA48::ProcessIL(const std::string& seq)
{
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
void SerialConnECMA48::ProcessREP(const std::string& seq)
{
}
void SerialConnECMA48::ProcessRM(const std::string& seq)
{
}
void SerialConnECMA48::ProcessSACS(const std::string& seq)
{
}
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
    mEcma48Funcs[ECMA48_CHA ] = [=](const std::string& seq) { ProcessCHA(seq); };
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
    // Trivial handlers
    mEcma48TrivialHandlers["[H"] = [=]() {
        mVx = 0;
        mVy = 0;
    };
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

void SerialConnECMA48::ProcessControlSeq(const std::string& seq, int seq_type)
{
    // for faster, we forward the seq_type as accurately as we could
    if (seq_type == ECMA48_Trivial) ProcessEcma48Trivial(seq); else {
        if (seq_type > ECMA48_Trivial && seq_type < ECMA48_SeqType_Endup) {
            auto it = mEcma48Funcs.find(seq_type);
            if (it != mEcma48Funcs.end()) {
                it->second(seq);
            }
        } else {
            Superclass::ProcessControlSeq(seq, seq_type);
        }
    }
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
