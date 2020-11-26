//
// (c) 2020 chiv
//
#ifndef _comxd_SerialIo_h_
#define _comxd_SerialIo_h_

#include "Action.h"
#include <string>
#include <vector>
#include <list>
// interface class
class SerialIo {
public:
    SerialIo();
    virtual ~SerialIo();
    //
    virtual bool Start() { return true; }
    /// Was data available?
    /// @return
    ///  - <0 Error, device was corrupted
    ///    =0 Nothing
    ///    >0 Number of bytes available
    virtual int Available() const = 0;
    virtual size_t Read(unsigned char* buff, size_t sz) = 0;
    virtual size_t Write(const unsigned char* buff, size_t sz) = 0;
    virtual std::string DeviceName() const = 0;
    // help
    std::vector<unsigned char> ReadRaw(size_t sz);
    std::string Read(size_t sz);
    size_t Write(const std::string& d);
    size_t Write(const std::vector<unsigned char>& d);
    //
    const std::list<UsrAction>& GetActions() const { return mUsrActions; }
    //
protected:
    std::list<UsrAction> mUsrActions;
};

#endif
