//
// (c) 2020 chiv
//
#include "terminal_rc.h"
#include "TextCodecsDialog.h"
#include "CodecFactory.h"


TextCodecsDialog::TextCodecsDialog(const char* name)
{
    CtrlLayout(*this);
    //
    this->Title(t_("Select a text codec"));
    this->Icon(terminal::text_codec());
    //
    auto codec_names = CodecFactory::Inst()->GetSupportedCodecNames();
    for (size_t k = 0; k < codec_names.size(); ++k) {
        mCodecs.Add(codec_names[k]);
    }
    mCodecs.SetData(name);
    //
    this->Acceptor(mOk, IDOK).Rejector(mCancel, IDCANCEL);
}

bool TextCodecsDialog::Key(Upp::dword key, int count)
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

TextCodecsDialog::~TextCodecsDialog()
{
}

Upp::String TextCodecsDialog::GetCodecName() const
{
    return mCodecs.Get();
}
