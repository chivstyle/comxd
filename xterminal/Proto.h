//
// (c) 2020 chiv
//
#ifndef _proto_h
#define _proto_h

#include <CtrlLib/CtrlLib.h>
#include <list>
#include <vector>

class SerialConn;
class SerialIo;
class Proto {
public:
    Proto(SerialConn*);
    virtual ~Proto();
    //
    const std::string& GetName() const { return mName; }
    void SetName(const char* name) { mName = name; }
    //
    virtual std::string GetDescription() const { return ""; }
    enum TransmitError {
        T_FAILED = -1,               // Failed to transmit
        T_NOT_SUPPORTED = -2         // Does not support transmit
    };
    virtual bool SupportTransmitData() const { return false; }
    virtual bool SupportTransmitFile() const { return false; }
    // typical behavior: pop up a dialog to show the transmit progress.
    // please invoke this routine from GUI thread
    // return <= 0 failed, T_NOT_SUPPORTED, T_FAILED, =0 No operations
    //        > 0  bytes transmitted
    virtual int TransmitData(const void* input, size_t input_size, std::string& errmsg)
    {
        (void)input;
        (void)input_size;
        (void)errmsg;
        return T_NOT_SUPPORTED;
    };
    // typical behavior : pop up a dialog to allow the user to select some file(s) to transmit
    virtual int TransmitFile() { return T_NOT_SUPPORTED; };
    //
    Upp::Event<Upp::Bar&> WhenUsrBar;
    //
protected:
    std::string mName;
    SerialConn* mConn;
};

#endif
