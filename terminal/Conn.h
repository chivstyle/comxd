#ifndef _comxd_Conn_h_
#define _comxd_Conn_h_

#include <CtrlLib/CtrlLib.h>
#include <memory>
#include <functional>
#include <list>

#include "SerialIo.h"
#include "Codec.h"
#include "Action.h"

class SerialConn : public Upp::TopWindow {
public:
    explicit SerialConn(std::shared_ptr<SerialIo> io);
    SerialConn() {}
    virtual ~SerialConn() {}
    // start the conn
    virtual bool Start() { return true; }
    //
    void SetCodec(const char* codec_name);
    //
    Upp::String ConnName() const
    {
        return mIo->DeviceName();
    }
    //
    const std::list<UsrAction>& GetActions() const
    {
        return mUsrActions;
    }
    //
    SerialIo* GetIo() const { return mIo.get(); }
    Codec* GetCodec() const { return mCodec.get(); }
    // terminal size changed
    Upp::Event<Upp::Size> WhenSizeChanged;
protected:
    std::shared_ptr<SerialIo> mIo;
    std::shared_ptr<Codec> mCodec;
    std::list<UsrAction> mUsrActions;
};

#endif
