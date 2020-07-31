//
// (c) 2020 chiv
//
#ifndef _comxd_SerialConnXterm_h_
#define _comxd_SerialConnXterm_h_

#include "SerialConnECMA48.h"

class SerialConnXterm : public SerialConnECMA48 {
public:
    using Superclass = SerialConnECMA48;
    SerialConnXterm(std::shared_ptr<serial::Serial> serial);
    virtual ~SerialConnXterm();
protected:
    virtual int IsControlSeq(const std::string& seq);
    void InstallXtermControlSeqHandlers();
};

#endif
