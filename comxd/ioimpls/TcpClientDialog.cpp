//
// (c) 2020 chiv
//
#include "resource.h"
#include "TcpClientDialog.h"
#include "CodecFactory.h"
#include "ConnFactory.h"
#include "ConnCreateFactory.h"

#define CONFIG_MAX_RECENT_RECS_NUMBER        10

using namespace Upp;

namespace {
class __class_to_create_conn_tcp {
public:
    __class_to_create_conn_tcp()
    {
        ConnCreateFactory::Inst()->RegisterInstanceFunc(TcpClient::kDeviceType, "TCP Client", [=]() { return comxd::tcp_client(); }, [=]() {
            TcpClientDialog d;
            return d.RequestConn();
        });
    }
};
__class_to_create_conn_tcp __tcp_conn_create;
}
// load history
std::deque<TcpClientDialog::TcpClientInfo> TcpClientDialog::GetRecentTcpClientInfos(int count) const
{
    std::deque<TcpClientInfo> out;
    auto conf = Upp::GetHomeDirectory() + "/.comxd/recent_tcp_clients.json";
    Upp::Value root = Upp::ParseJSON(Upp::LoadFile(conf));
    if (!root.IsNull()) {
        if (root.GetType() == Upp::VALUEARRAY_V) {
            int item_count = root.GetCount();
            int item_from = item_count > count ? item_count - count : 0;
            for (int i = item_from; i < item_count; ++i) {
                if (Find(out, ~root[i]["Host"])) continue;
                int port = 22;
                if (!root[i]["Port"].IsNull()) {
                    port = (int)root[i]["Port"];
                }
                out.push_back({~root[i]["Host"], port});
            }
        }
    }
    return out;
}
const TcpClientDialog::TcpClientInfo* TcpClientDialog::Find(const std::deque<TcpClientDialog::TcpClientInfo>& infos, const String& host) const
{
    for (size_t k = 0; k < infos.size(); ++k) {
        if (infos[k].Host == host)
            return &infos[k];
    }
    return nullptr;
}

const TcpClientDialog::TcpClientInfo* TcpClientDialog::FindRecent(const String& host) const
{
    return Find(mRecents, host);
}

void TcpClientDialog::SaveRecentTcpClientInfos() const
{
    JsonArray out;
    const std::deque<TcpClientDialog::TcpClientInfo>& infos = mRecents;
    for (size_t k = 0; k < infos.size(); ++k) {
        out << Json("Host", infos[k].Host)("Port", infos[k].Port);
    }
    auto conf = Upp::GetHomeDirectory() + "/.comxd/recent_tcp_clients.json";
    Upp::SaveFile(conf, out.ToString());
}

void TcpClientDialog::AddRecentTcpClientInfo(const String& host, int port)
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
    mRecents.emplace_back(TcpClientInfo{host, port});
}

TcpClientDialog::TcpClientDialog()
    : mConn(nullptr)
{
    Icon(comxd::tcp_client()).Title("TCP Client");
    //
    mPort.SetData(22);
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
    mRecents = GetRecentTcpClientInfos(CONFIG_MAX_RECENT_RECS_NUMBER);
    for (size_t i = 0; i < mRecents.size(); ++i) {
        mHost.AddList(mRecents[i].Host);
    }
    if (!mRecents.empty()) { // set last one
        mHost.SetData(mRecents.rbegin()->Host);
        mPort.SetData(mRecents.rbegin()->Port);
    }
    mHost.WhenSelect = [=]() {
        auto item = this->FindRecent(~mHost);
        if (item) {
            mHost.SetData(item->Host);
            mPort.SetData(item->Port);
        }
    };
    //
    CtrlLayout(*this);
    //
    Rejector(mBtnCancel, IDOK);
    //
    mBtnOk.WhenAction = [=]() { CreateConn(); };
}

bool TcpClientDialog::Key(Upp::dword key, int count)
{
    dword flags = K_CTRL | K_ALT | K_SHIFT;
    dword d_key = key & ~(flags | K_KEYUP); // key with delta
    flags = key & flags;
    if (key & Upp::K_KEYUP) {
        if (flags == 0 && d_key == Upp::K_ESCAPE) {
            Close();
            return true;
        }
        if (flags == 0 && d_key == 0xd) {
            mBtnOk.WhenAction();
        }
    }
    return TopWindow::Key(key, count);
}
// Invoke this routine from main thread (GUI thread)
static inline bool WaitTcpConnection(TcpSocket* s, int timeout)
{
	s->Timeout(16);
	while (timeout > 0) {
		auto t1 = std::chrono::steady_clock::now();
		if (s->WaitConnect()) {
		    break;
		}
		Ctrl::ProcessEvents();
		auto t2 = std::chrono::steady_clock::now();
		timeout -= (int)std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	}
	return timeout > 0;
}

bool TcpClientDialog::RequestReconnect(TcpClient* sc)
{
    TcpSocket* tcp = sc->Tcp();
	mBtnOk.WhenAction = [=]() { AcceptBreak(IDOK); };
	mCodecs.Clear(); mCodecs.Disable();
	mTypes.Clear(); mTypes.Disable();
	mHost.Disable();
	mHost.SetData(sc->Host());
	mPort.SetData(sc->Port());
	return Run(true) == IDOK;
}

bool TcpClientDialog::Reconnect(TcpClient* sc)
{
    TcpSocket* tcp = sc->Tcp();
    auto title = GetTitle();
    Title(t_("Connecting...")).Disable();
    Upp::String host = ~mHost;
    if (tcp->Timeout(2000).Connect(host, ~mPort)) {
        if (!WaitTcpConnection(tcp, 2000)) {
            PromptOK(Upp::DeQtf(t_("Can't establish the connection")));
            return false;
        }
        return true;
    } else {
        PromptOK(Upp::DeQtf(tcp->GetErrorDesc()));
    }
    return false;
}

void TcpClientDialog::CreateConn()
{
    std::shared_ptr<TcpSocket> tcp = std::make_shared<TcpSocket>();
    auto title = GetTitle();
    Title(t_("Connecting...")).Disable();
    Upp::String host = ~mHost;
    if (tcp->Timeout(2000).Connect(host, ~mPort)) {
        try {
            if (!WaitTcpConnection(tcp.get(), 2000)) {
                throw String(t_("Can't establish the connection"));
            }
            auto port = std::make_shared<TcpClient>(tcp, ~mHost, ~mPort);
            auto conn = ConnFactory::Inst()->CreateInst(~mTypes, port);
            port->Start();
            conn->SetCodec(mCodecs.GetData().ToString());
            conn->Start();
            //
            mConn = conn;
            // add to recent list
            this->AddRecentTcpClientInfo(~mHost, ~mPort);
            this->SaveRecentTcpClientInfos();
            //
            this->AcceptBreak(IDOK);
        } catch (const String& desc) {
            PromptOK(Upp::DeQtf(desc));
        }
    } else {
        PromptOK(Upp::DeQtf(tcp->GetErrorDesc()));
    }
    Title(title).Enable();
}

SerialConn* TcpClientDialog::RequestConn()
{
    if (Run(true) == IDOK) {
        return mConn;
    }
    return nullptr;
}
