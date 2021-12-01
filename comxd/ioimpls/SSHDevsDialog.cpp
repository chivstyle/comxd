//
// (c) 2020 chiv
//
#include "resource.h"
#include "SSHDevsDialog.h"
#include "CodecFactory.h"
#include "ConnFactory.h"
#include "SSHPort.h"
#include "ConnCreateFactory.h"

#define CONFIG_MAX_RECENT_RECS_NUMBER        10

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
// load history
std::deque<SSHDevsDialog::SSHDevInfo> SSHDevsDialog::GetRecentSSHDevInfos(int count) const
{
    std::deque<SSHDevInfo> out;
    auto conf = Upp::GetHomeDirectory() + "/.comxd/recent_ssh_devs.json";
    Upp::Value root = Upp::ParseJSON(Upp::LoadFile(conf));
    if (!root.IsNull()) {
        if (root.GetType() == Upp::VALUEARRAY_V) {
            int item_count = root.GetCount();
            int item_from = item_count > count ? item_count - count : 0;
            for (int i = item_from; i < item_count; ++i) {
                if (Find(out, ~root[i]["Host"])) continue;
                out.push_back({~root[i]["Host"], ~root[i]["User"], ~root[i]["Type"], ~root[i]["Code"]});
            }
        }
    }
    return out;
}
const SSHDevsDialog::SSHDevInfo* SSHDevsDialog::Find(const std::deque<SSHDevsDialog::SSHDevInfo>& infos, const String& host) const
{
    for (size_t k = 0; k < infos.size(); ++k) {
        if (infos[k].Host == host)
            return &infos[k];
    }
    return nullptr;
}

const SSHDevsDialog::SSHDevInfo* SSHDevsDialog::FindRecent(const String& host) const
{
    return Find(mRecents, host);
}

void SSHDevsDialog::SaveRecentSSHDevInfos() const
{
    JsonArray out;
    const std::deque<SSHDevsDialog::SSHDevInfo>& infos = mRecents;
    for (size_t k = 0; k < infos.size(); ++k) {
        out << Json("Host", infos[k].Host)("User", infos[k].User)("Type", infos[k].Type)("Code", infos[k].Code);
    }
    auto conf = Upp::GetHomeDirectory() + "/.comxd/recent_ssh_devs.json";
    Upp::SaveFile(conf, out.ToString());
}

void SSHDevsDialog::AddRecentSSHDevInfo(const String& host, const String& user, const String& type, const String& code)
{
    for (auto it = mRecents.begin(); it != mRecents.end(); ) {
        if (it->Host == host) {
            it = mRecents.erase(it);
        } else ++it;
    }
    //
    if (mRecents.size() >= CONFIG_MAX_RECENT_RECS_NUMBER) {
        mRecents.pop_front();
    }
    mRecents.emplace_back(SSHDevInfo{host, user, type, code});
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
    mRecents = GetRecentSSHDevInfos(CONFIG_MAX_RECENT_RECS_NUMBER);
    for (size_t i = 0; i < mRecents.size(); ++i) {
        mHost.AddList(mRecents[i].Host);
    }
    if (!mRecents.empty()) { // set last one
        mHost.SetData(mRecents.rbegin()->Host);
        mUser.SetData(mRecents.rbegin()->User);
        mTypes.SetData(mRecents.rbegin()->Type);
        mCodecs.SetData(mRecents.rbegin()->Code);
    }
    mHost.WhenSelect = [=]() {
        auto item = this->FindRecent(~mHost);
        if (item) {
            mHost.SetData(item->Host);
            mUser.SetData(item->User);
            mTypes.SetData(item->Type);
            mCodecs.SetData(item->Code);
        }
    };
    //
    CtrlLayout(*this);
    //
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
            // add to recent list
            this->AddRecentSSHDevInfo(~mHost, ~mUser, ~mTypes, ~mCodecs);
            this->SaveRecentSSHDevInfos();
            //
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
