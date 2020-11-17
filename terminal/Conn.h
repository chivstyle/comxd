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
    explicit SerialConn(std::shared_ptr<SerialIo> serial);
    SerialConn() {}
    virtual ~SerialConn() {}
    //
    void SetCodec(const char* codec_name);
    //
    Upp::String ConnName() const
    {
        return mSerial->DeviceName();
    }
    //
    const std::list<UsrAction>& GetActions() const
    {
        return mUsrActions;
    }
    //
    SerialIo* GetSerial() const { return mSerial.get(); }
    Codec* GetCodec() const { return mCodec.get(); }
    // Event.
    Upp::Event<Upp::Size> WhenSizeChanged;
protected:
    std::shared_ptr<SerialIo> mSerial;
    std::shared_ptr<Codec> mCodec;
    std::list<UsrAction> mUsrActions;
};

#endif
