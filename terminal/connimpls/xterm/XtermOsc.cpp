/*!
// (c) 2021 chiv
//
// Process OSC string here
*/
#include "SerialConnXterm.h"

using namespace Upp;

static inline Color XParseColor(const std::string& spec)
{
    std::string ps[4];
    int idx = 0;
    SplitString(spec.c_str(), ":/", [=, &idx, &ps](const char* token) {
        if (idx < 4)
            ps[idx++] = token;
    });
    if (ps[0] == "rgbi") {
        return Color((int)(255 * atof(ps[1].c_str())),
                     (int)(255 * atof(ps[2].c_str())),
                     (int)(255 * atof(ps[3].c_str())));
    } else if (ps[0] == "rgb") {
        return Color((int)strtol(ps[1].c_str(), nullptr, 16),
                     (int)strtol(ps[2].c_str(), nullptr, 16),
                     (int)strtol(ps[3].c_str(), nullptr, 16));
    } else if (ps[0] == "CIEXYZ") {
    } else if (ps[0] == "CIEuvY") {
    } else if (ps[0] == "CIExyY") {
    } else if (ps[0] == "CIELab") {
    } else if (ps[0] == "CIELuv") {
    } else if (ps[0] == "TekHVC") {
    }
    // TODO: Implement color spec
    return Color(rand() % 255, rand() % 255, rand() % 255);
}
static inline std::string XParseColor(const Color& color)
{
    std::string spec = "rgb:";
    spec += std::to_string(color.GetR()) + ":";
    spec += std::to_string(color.GetG()) + ":";
    spec += std::to_string(color.GetB());
    return spec;
}

void SerialConnXterm::ProcessXTOSC(const std::string& p)
{
    std::string ps[16];
    int idx = 0;
    SplitString(p.data(), ";", [=, &idx, &ps](const char* token) {
        if (idx < 16)
            ps[idx] = token;
        idx++;
    });
    if (idx == 2) {
        switch (atoi(ps[0].c_str())) {
        case 0: // change Icon Name And Window Title
        case 1: // change Icon Name
        case 2: // change window Title
            SetTitle(ps[1]);
            PostCallback([=] {
                this->WhenTitle(GetTitle());
            });
            break;
        case 3: if (1) {
            const char* pv[2] = {"", ""};
            int pi = 0;
            SplitString(std::move(ps[1]), "=", [=, &pi, &pv](const char* token) {
                if (pi < 2)
                    pv[pi] = token;
                pi++;
            });
            if (pi > 0) {
                mRcs[pv[0]] = pv[1];
            }
        } break;
        case 10: if (1) {
            auto it = mRcs.find("allowColorOps");
            if (it != mRcs.end() && it->second != "1" || it == mRcs.end())
                break;
            if (ps[1] == "?") {
                std::string resp = std::string("\x1b]10;") + XParseColor(mColorTbl.GetColor(VTColorTable::kColorId_Texts));
                Put(resp);
            } else {
                mColorTbl.SetColor(VTColorTable::kColorId_Texts, XParseColor(ps[1]));
            }
        } break;
        case 11: if (1) {
            auto it = mRcs.find("allowColorOps");
            if (it != mRcs.end() && it->second != "1" || it == mRcs.end())
                break;
            if (ps[1] == "?") {
                std::string resp = std::string("\x1b]10;") + XParseColor(mColorTbl.GetColor(VTColorTable::kColorId_Paper));
                Put(resp);
            } else {
                mColorTbl.SetColor(VTColorTable::kColorId_Paper, XParseColor(ps[2]));
            }
        } break;
        // 12-19, we ignore them.
        // 50,51, .etc we do not support Font ops, you can change font from VTOptionsDialog.
        case 104: // reset color
            mColorTbl.ResetColor(mColorTbl.FindNearestColorId(atoi(ps[1].c_str())));
            break;
        // 105,106 we do not support Xterm spec color
        case 110:
            mColorTbl.ResetColor(VTColorTable::kColorId_Texts);
            break;
        case 111:
            mColorTbl.ResetColor(VTColorTable::kColorId_Paper);
            break;
        // 112-119, we ignore them
        }
    } else if (idx == 3) {
        switch (atoi(ps[0].c_str())) {
        case 4:
            mColorTbl.SetColor(mColorTbl.FindNearestColorId(atoi(ps[1].c_str())), XParseColor(ps[2]));
            break;
        // 5,6, we do not support Xterm spec color
        }
    }
}
