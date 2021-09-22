/*!
// (c) 2021 chiv
//
*/
#pragma once

#include <CtrlLib/CtrlLib.h>

class TransmitProgressDialog : public WithTransmitProgressDialog<Upp::TopWindow> {
public:
    typedef TransmitProgressDialog CLASSNAME;
    TransmitProgressDialog();
    void SetTotal(size_t total);
    // count - [0, total]
    // tx_rate - in KiB
    void Update(size_t count, double tx_rate);
private:
    size_t mTotal;
};
