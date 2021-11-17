#ifndef _comxd_Conn_h_
#define _comxd_Conn_h_

#include <CtrlLib/CtrlLib.h>
#include <functional>
#include <list>
#include <memory>

#include "Action.h"
#include "Codec.h"
#include "SerialIo.h"

class SerialConn : public Upp::ParentCtrl {
public:
    explicit SerialConn(std::shared_ptr<SerialIo> io);
    SerialConn() { }
    virtual ~SerialConn() { }
    // start the works
    virtual bool Start() { return true; }
    // stop the works
    virtual void Stop() {}
    //
    void SetCodec(const char* codec_name);
    //
    Upp::String ConnName() const
    {
        return mIo->DeviceName();
    }
    void SetConnDescription(const Upp::String& desc) { mDesc = desc; }
    const Upp::String& GetConnDescription() const { return mDesc; }
    void SetTitle(const Upp::String& title) { mTitle = title; }
    const Upp::String& GetTitle() const { return mTitle; }
    //
    SerialIo* GetIo() const { return mIo.get(); }
    Codec* GetCodec() const { return mCodec.get(); }
    // terminal size changed
    Upp::Event<Upp::Size> WhenSizeChanged;
    Upp::Event<Upp::Bar&> WhenUsrBar;
    Upp::Event<Upp::String> WhenTitle;
    Upp::Event<Upp::String> WhenWarning;
protected:
    std::shared_ptr<SerialIo> mIo;
    std::shared_ptr<Codec> mCodec;
    //
private:
    Upp::String mDesc;
    Upp::String mTitle;
};

#endif
