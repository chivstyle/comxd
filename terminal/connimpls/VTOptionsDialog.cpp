//
// (c) 2020 chiv
//
#include "terminal_rc.h"
#include "VTOptionsDialog.h"

using namespace Upp;
//
struct FontFaceDisplay : Display {
    virtual void Paint(Draw& w, const Rect& r, const Value& q,
                       Color ink, Color paper, dword style) const
    {
        Font fnt = Font(q, 14);
        String txt = Font::GetFaceName(q);
        w.DrawRect(r, paper);
        w.DrawText(r.left + 2, r.top + (r.Height() - GetTextSize(txt, fnt).cy) / 2, txt, fnt, ink);
    }
};

static const int kFontSizes[] = {
    8,9,10,11,12,
    14,16,18,20,22,
    24,28,36,42,48,
    64,72
};

VTOptionsDialog::VTOptionsDialog()
{
    Upp::CtrlLayout(*this);
    // set icon
    this->Icon(terminal::vt_options());
    // list fixed-width fonts
    mFontList.SetDisplay(Single<FontFaceDisplay>());
    for(int i = 0; i < Font::GetFaceCount(); i++) {
        // set a fixed-pitch font as default
        if (Font::GetFaceInfo(i) & Font::FIXEDPITCH) {
            mFontList.Add(i);
        }
    }
    mFontList.SetIndex(0);
    for (int i = 0; i < ARRAYSIZE(kFontSizes); ++i) {
        mFontSize.AddList(kFontSizes[i]);
    }
    mFontSize.SetData(11);
    //
    PreviewFont();
    //
    mLinesBufferSize.SetData(5000);
    //
    InstallActions();
    //
    Acceptor(mOk, IDOK).Rejector(mCancel, IDCANCEL);
}

VTOptionsDialog::~VTOptionsDialog()
{
}

bool VTOptionsDialog::Key(Upp::dword key, int count)
{
	dword flags = K_CTRL | K_ALT | K_SHIFT;
    dword d_key = key & ~(flags | K_KEYUP); // key with delta
    flags = key & flags;
    if (key & Upp::K_KEYUP) {
        if (flags == 0 && d_key == Upp::K_ESCAPE) {
            Close();
            return true;
        }
    }
	return TopWindow::Key(key, count);
}

void VTOptionsDialog::InstallActions()
{
    mFontList.WhenAction = [=]() {
        int sel_value = 12;
        if (mFontSize.GetCount()) {
            sel_value = mFontSize.GetData().To<int>();
        }
        mFontSize.ClearList();
        Font font(mFontList.Get(), sel_value);
        if (font.IsScaleable()) {
            for (int i = 0; i < ARRAYSIZE(kFontSizes); ++i) {
                mFontSize.AddList(kFontSizes[i]);
            }
            mFontSize.SetData(sel_value);
        } else {
            mFontSize.SetData(font.GetHeight());
        }
        PreviewFont();
    };
    mFontSize.WhenAction = [=]() {
        PreviewFont();
    };
}

Font VTOptionsDialog::DefaultFont()
{
    int sel = -1;
    for(int i = 0; i < Font::GetFaceCount(); i++) {
        // set a fixed-pitch font as default
        if (sel == -1 && (Font::GetFaceInfo(i) & Font::FIXEDPITCH)) {
            sel = i;
        }
    }
    if (sel < 0) sel = 0;
    return Font(sel, 12);
}

void VTOptionsDialog::PreviewFont()
{
    Font font(mFontList.Get(), mFontSize.GetData().To<int>());
    mFontPreview.SetInk(mTextsColor.GetData());
    mFontPreview.SetFont(font).SetText("Hello,世界!");
}

void VTOptionsDialog::SetOptions(const Options& options)
{
    int face = options.Font.GetFace();
    mFontList.Set(face);
    mFontSize.SetData(options.Font.GetHeight());
    mPaperColor.SetData(options.PaperColor);
    mTextsColor.SetData(options.TextsColor);
    mLinesBufferSize.SetData(options.LinesBufferSize);
    mFontList.WhenAction();
    //
    PreviewFont();
}

VTOptionsDialog::Options VTOptionsDialog::GetOptions() const
{
    Options options;
    options.Font = Font(mFontList.Get(), mFontSize.GetData().To<int>());
    options.PaperColor = mPaperColor.GetData();
    options.TextsColor = mTextsColor.GetData();
    options.LinesBufferSize = mLinesBufferSize.GetData();
    return options;
}

bool VTOptionsDialog::Accept()
{
    return true;
}
