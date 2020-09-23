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
    this->Title(t_("Text Codec"));
    this->Icon(terminal::text_codec());
    //
    auto codecs = CodecFactory::Inst()->GetSupportedCodecs();
    for (size_t k = 0; k < codecs.size(); ++k) {
        mCodecs.Add(codecs[k].c_str());
    }
    mCodecs.SetData(name);
    //
    this->Acceptor(mOk, IDOK).Rejector(mCancel, IDCANCEL);
}

TextCodecsDialog::~TextCodecsDialog()
{
}

Upp::String TextCodecsDialog::GetCodecName() const
{
    return mCodecs.Get().ToString();
}
