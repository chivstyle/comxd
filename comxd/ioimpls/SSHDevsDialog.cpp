//
// (c) 2020 chiv
//
#include "resource.h"
#include "SSHDevsDialog.h"
#include "CodecFactory.h"
#include "ConnFactory.h"
#include "SSHPort.h"
#include "ConnCreateFactory.h"

using namespace Upp;

namespace {
class __class_to_create_conn_ssh {
public:
    __class_to_create_conn_ssh()
    {
        ConnCreateFactory::Inst()->RegisterInstanceFunc("SSH", "SSH Shell", comxd::new_ssh(), [=]() {
            SSHDevsDialog d;
            return d.RequestConn();
        });
    }
};
__class_to_create_conn_ssh __ssh_conn_create;
}

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

    Rejector(mBtnCancel, IDCANCEL);
    //
    mBtnOk.WhenAction = [=]() { CreateConn(); };
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

void SSHDevsDialog::CreateConn()
{
    std::shared_ptr<SshSession> session = std::make_shared<SshSession>();
    session->WhenWait = [=]() {
        if (IsMainThread())
            ProcessEvents();
    };
    auto title = GetTitle();
    Title(t_("Connecting...")).Disable();
    if (session->Timeout(5000).Connect(~mHost, ~mPort, ~mUser, ~mPassword)) {
        try {
            auto port = std::make_shared<SSHPort>(session, ~mHost,
                ConnFactory::Inst()->GetConnType(~mTypes));
            auto conn = ConnFactory::Inst()->CreateInst(~mTypes, port);
            port->Start();
            conn->WhenSizeChanged = [=](const Size& csz) {
                port->SetConsoleSize(csz);
            };
            conn->SetCodec(mCodecs.GetData().ToString());
            conn->Start();
            //
            mConn = conn;
            this->AcceptBreak(IDOK);
            //
            return;
        } catch (const String& desc) {
            PromptOK(Upp::DeQtf(desc));
        }
    } else {
        PromptOK(Upp::DeQtf(session->GetErrorDesc()));
    }
    Title(title).Enable();
}

SerialConn* SSHDevsDialog::RequestConn()
{
    if (Run(true) == IDOK) {
        return mConn;
    }
    return nullptr;
}
