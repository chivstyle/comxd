//
// (c) 2020 chiv
//
#ifndef _comxd_SerialConnECMA48_h_
#define _comxd_SerialConnECMA48_h_

#include "SerialConnVT102.h"

class SerialConnECMA48 : public SerialConnVT102 {
public:
    using Superclass = SerialConnVT102;
    SerialConnECMA48(std::shared_ptr<serial::Serial> serial);
    virtual ~SerialConnECMA48();
protected:
    virtual int IsControlSeq(const std::string& seq);
    virtual void ProcessAttr(const std::string& attr_code);
    void InstallECMA48ControlSeqHandlers();
};


#endif
