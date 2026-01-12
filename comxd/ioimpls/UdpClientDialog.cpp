//
// (c) 2020 chiv
//
#include "resource.h"
#include "UdpClientDialog.h"
#include "CodecFactory.h"
#include "ConnFactory.h"
#include "ConnCreateFactory.h"

#define CONFIG_MAX_RECENT_RECS_NUMBER        10

using namespace Upp;

namespace {
class __class_to_create_conn_udp {
public:
    __class_to_create_conn_udp()
    {
        ConnCreateFactory::Inst()->RegisterInstanceFunc(UdpClient::kDeviceType, "UDP Client", [=]() { return comxd::udp_client(); }, [=]() {
            UdpClientDialog d;
            return d.RequestConn();
        });
    }
};
__class_to_create_conn_udp __udp_conn_create;
}
// load history
std::deque<UdpClientDialog::UdpClientInfo> UdpClientDialog::GetRecentUdpClientInfos(int count) const
{
    std::deque<UdpClientInfo> out;
    auto conf = Upp::GetHomeDirectory() + "/.comxd/recent_udp_clients.json";
    Upp::Value root = Upp::ParseJSON(Upp::LoadFile(conf));
    if (!root.IsNull()) {
        if (root.GetType() == Upp::VALUEARRAY_V) {
            int item_count = root.GetCount();
            int item_from = item_count > count ? item_count - count : 0;
            for (int i = item_from; i < item_count; ++i) {
                if (Find(out, ~root[i]["Host"])) continue;
                int dest_port = 22;
                int listen_port = 0;
                if (!root[i]["DestPort"].IsNull()) {
                    dest_port = (int)root[i]["DestPort"];
                }
                if (!root[i]["ListenPort"].IsNull()) {
                    listen_port = (int)root[i]["ListenPort"];
                }
                String local;
                if (!root[i]["Local"].IsNull()) {
                    local = root[i]["Local"];
                }
                out.push_back({~root[i]["Host"], local, dest_port, listen_port});
            }
        }
    }
    return out;
}
const UdpClientDialog::UdpClientInfo* UdpClientDialog::Find(const std::deque<UdpClientDialog::UdpClientInfo>& infos, const String& host) const
{
    for (size_t k = 0; k < infos.size(); ++k) {
        if (infos[k].Host == host)
            return &infos[k];
    }
    return nullptr;
}

const UdpClientDialog::UdpClientInfo* UdpClientDialog::FindRecent(const String& host) const
{
    return Find(mRecents, host);
}

void UdpClientDialog::SaveRecentUdpClientInfos() const
{
    JsonArray out;
    const std::deque<UdpClientDialog::UdpClientInfo>& infos = mRecents;
    for (size_t k = 0; k < infos.size(); ++k) {
        out << Json("Host", infos[k].Host)("Local", infos[k].Local)("DestPort", infos[k].DestPort)("ListenPort", infos[k].ListenPort);
    }
    auto conf = Upp::GetHomeDirectory() + "/.comxd/recent_udp_clients.json";
    Upp::SaveFile(conf, out.ToString());
}

void UdpClientDialog::AddRecentUdpClientInfo(const String& host, const String& local, int listen_port, int dest_port)
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
    mRecents.emplace_back(UdpClientInfo{host, local, dest_port, listen_port});
}

UdpClientDialog::UdpClientDialog()
    : mConn(nullptr)
{
    Icon(comxd::udp_client()).Title("UDP Client");
    //
    mPort.SetData(22);
    mListenPort.SetData(0);
    mLocal.SetData("0.0.0.0");
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
    mRecents = GetRecentUdpClientInfos(CONFIG_MAX_RECENT_RECS_NUMBER);
    for (size_t i = 0; i < mRecents.size(); ++i) {
        mHost.AddList(mRecents[i].Host);
    }
    if (!mRecents.empty()) { // set last one
        mHost.SetData(mRecents.rbegin()->Host);
        mLocal.SetData(mRecents.rbegin()->Local);
        mPort.SetData(mRecents.rbegin()->DestPort);
        mListenPort.SetData(mRecents.rbegin()->ListenPort);
    }
    mHost.WhenSelect = [=]() {
        auto item = this->FindRecent(~mHost);
        if (item) {
            mHost.SetData(item->Host);
            mLocal.SetData(item->Local);
            mPort.SetData(item->DestPort);
            mListenPort.SetData(item->ListenPort);
        }
    };
    //
    CtrlLayout(*this);
    //
    Rejector(mBtnCancel, IDOK);
    //
    mBtnOk.WhenAction = [=]() { CreateConn(); };
}

bool UdpClientDialog::Key(Upp::dword key, int count)
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

bool UdpClientDialog::RequestReconnect(UdpClient* sc)
{
	mBtnOk.WhenAction = [=]() { AcceptBreak(IDOK); };
	mCodecs.Clear(); mCodecs.Disable();
	mTypes.Clear(); mTypes.Disable();
	mHost.Disable();
	mHost.SetData(sc->Host());
	mPort.SetData(sc->DestPort());
	mLocal.SetData(sc->Local());
	mListenPort.SetData(sc->ListenPort());
	return Run(true) == IDOK;
}

bool UdpClientDialog::Reconnect(UdpClient* sc)
{
	sc->Set(~mHost, ~mLocal, ~mListenPort, ~mPort);
	return true;
}

void UdpClientDialog::CreateConn()
{
	try {
		auto port = std::make_shared<UdpClient>(~mHost, ~mLocal, ~mListenPort, ~mPort);
        auto conn = ConnFactory::Inst()->CreateInst(~mTypes, port);
        port->Start();
        conn->SetCodec(mCodecs.GetData().ToString());
        conn->Start();
        //
        mConn = conn;
        // add to recent list
        this->AddRecentUdpClientInfo(~mHost, ~mLocal, ~mListenPort, ~mPort);
        this->SaveRecentUdpClientInfos();
        //
        this->AcceptBreak(IDOK);
	} catch (const std::exception& ex) {
		Upp::PromptOK(Upp::DeQtf(ex.what()));
	}
}

SerialConn* UdpClientDialog::RequestConn()
{
    if (Run(true) == IDOK) {
        return mConn;
    }
    return nullptr;
}
