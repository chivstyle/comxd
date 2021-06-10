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
    auto conn_names = ConnFactory::Inst()->GetSupportedConnNames();
    for (size_t k = 0; k < conn_names.size(); ++k) {
        mTypes.Add(conn_names[k]);
    }
    if (!conn_names.empty()) {
        mTypes.SetIndex(0);
    }
    // CodecFactory MUST have a UTF-8 codec.
    auto codec_names = CodecFactory::Inst()->GetSupportedCodecNames();
    for (size_t k = 0; k < codec_names.size(); ++k) {
        mCodecs.Add(codec_names[k]);
        if (codec_names[k] == "UTF-8" || codec_names[k] == "UTF8") {
            mCodecs.SetIndex((int)k);
        }
    }
	CtrlLayout(*this);
	
	this->Acceptor(mBtnOk, IDOK).Rejector(mBtnCancel, IDCANCEL);
}

bool SSHDevsDialog::Key(Upp::dword key, int count)
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

SerialConn* SSHDevsDialog::RequestConn()
{
	SerialConn* conn = nullptr;
	if (Run(true) == IDOK) {
		this->Disable(); // disable the dialog.
		std::shared_ptr<SshSession> session = std::make_shared<SshSession>();
		this->Title(t_("Connecting..."));
		session->WhenWait = [=]() { if (IsMainThread()) ProcessEvents(); };
		if (session->Timeout(2000).Connect(~mHost, ~mPort, ~mUser, ~mPassword)) {
			try {
				auto port = std::make_shared<SSHPort>(session, ~mHost,
					ConnFactory::Inst()->GetConnType(~mTypes));
				conn = ConnFactory::Inst()->CreateInst(~mTypes, port);
				port->Start();
				conn->WhenSizeChanged = [=](const Size& csz) {
					port->SetConsoleSize(csz);
				};
				conn->SetCodec(mCodecs.GetData().ToString());
				conn->Start();
			} catch (const String& desc) {
				PromptOK(Upp::DeQtf(desc));
			}
		} else {
			PromptOK(Upp::DeQtf(session->GetErrorDesc()));
		}
	}
	return conn;
}
