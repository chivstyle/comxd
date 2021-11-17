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
    void SetTotal(Upp::int64 total);
    // count - [0, total]
    // tx_rate - in KiB
    void Update(Upp::int64 count, double tx_rate);
private:
    Upp::int64 mTotal;
};
