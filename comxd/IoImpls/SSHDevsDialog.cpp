//
// (c) 2020 chiv
//
#include "resource.h"
#include "SSHDevsDialog.h"
#include "SSHPort.h"
#include "ConnFactory.h"
#include "CodecFactory.h"

using namespace Upp;

SSHDevsDialog::SSHDevsDialog()
{
	Icon(comxd::new_ssh()).Title("SSH");
	//
	mPort.SetData("22");
	mPassword.Password();
	// types
    auto types = ConnFactory::Inst()->GetSupportedConnTypes();
    for (size_t k = 0; k < types.size(); ++k) {
        mTypes.Add(types[k].c_str());
    }
    if (!types.empty()) {
        mTypes.SetIndex(0);
    }
    // CodecFactory MUST have a UTF-8 codec.
    auto codecs = CodecFactory::Inst()->GetSupportedCodecs();
    for (size_t k = 0; k < codecs.size(); ++k) {
        mCodecs.Add(codecs[k].c_str());
        if (codecs[k] == "UTF-8" || codecs[k] == "UTF8") {
            mCodecs.SetIndex((int)k);
        }
    }
	
	CtrlLayout(*this);
	
	this->Acceptor(mBtnOk, IDOK).Rejector(mBtnCancel, IDCANCEL);
}

SerialConn* SSHDevsDialog::RequestConn()
{
	if (Run(true) == IDOK) {
		try {
			auto port = std::make_shared<SSHPort>(mHost.GetData().ToString(), mPort,
				mUser.GetData().ToString(), mPassword.GetData().ToString());
			auto conn = ConnFactory::Inst()->CreateInst(mTypes.Get().ToString(), port);
			conn->WhenSizeChanged = [=](const Size& csz) {
				port->SetConsoleSize(csz);
			};
			conn->SetCodec(mCodecs.Get().ToString());
			return conn;
		} catch (const String& desc) {
			PromptOK(Upp::DeQtf(desc));
		}
	}
	return nullptr;
}
