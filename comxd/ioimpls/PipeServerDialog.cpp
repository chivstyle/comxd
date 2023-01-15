//
// (c) 2020 chiv
//
#ifdef _WIN32
#include "resource.h"
#include "PipeServerDialog.h"
#include "CodecFactory.h"
#include "ConnFactory.h"
#include "PipeServer.h"
#include "ConnCreateFactory.h"

using namespace Upp;

namespace {
class __class_to_create_conn_pipeserver {
public:
    __class_to_create_conn_pipeserver()
    {
        ConnCreateFactory::Inst()->RegisterInstanceFunc(NamedPipeServer::kDeviceType, "Pipe Server", [=]() { return comxd::pipe_server(); }, [=]() {
            PipeServerDialog d;
            return d.RequestConn();
        });
    }
};
__class_to_create_conn_pipeserver __pipeserver_conn_create;
}

PipeServerDialog::PipeServerDialog()
    : mConn(nullptr)
{
    Icon(comxd::pipe_server()).Title("Pipe Server");
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
    mRxBufferSize.SetData(128);
    mTxBufferSize.SetData(128);
    mTimeout.SetData(1000);
    mInstanceNb.SetData(1);
    //
    CtrlLayout(*this);
    //
    Rejector(mButtonCancel, IDCANCEL);
    //
    mButtonOk.WhenAction = [=]() { CreateConn(); };
}

bool PipeServerDialog::Key(Upp::dword key, int count)
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
            mButtonOk.WhenAction();
        }
    }
    return TopWindow::Key(key, count);
}

void PipeServerDialog::CreateConn()
{
    try {
        auto port = std::make_shared<NamedPipeServer>(~mName, ~mRxBufferSize, ~mTxBufferSize, ~mTimeout, ~mInstanceNb);
        auto conn = ConnFactory::Inst()->CreateInst(~mTypes, port);
        if (conn) {
            if (!port->Start()) {
                delete conn;
                throw std::runtime_error("can't start port");
            }
            conn->SetCodec(mCodecs.Get().ToString());
            conn->Start();
            mConn = conn;
            this->AcceptBreak(IDOK);
        } else {
            throw std::runtime_error("can't create conn");
        }
    } catch (const std::exception& ex) {
        PromptOK(DeQtf(ex.what()));
    }
}

SerialConn* PipeServerDialog::RequestConn()
{
    if (Run(true) == IDOK) {
        return mConn;
    }
    return nullptr;
}

#endif
