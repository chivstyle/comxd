//
// (c) 2020 chiv
//
#ifndef _comxd_SerialConnXterm_h_
#define _comxd_SerialConnXterm_h_

#include "SerialConnVT102.h"

class SerialConnXterm : public SerialConnVT102 {
public:
    SerialConnXterm(std::shared_ptr<serial::Serial> serial);
    virtual ~SerialConnXterm();
protected:
    virtual int IsControlSeq(const std::string& seq);
    void InstallXtermControlSeqHandlers();
};

#endif
