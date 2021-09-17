//
// (c) 2021 chiv
//
#include "terminal_rc.h"
#include "TransmitProgressDialog.h"

TransmitProgressDialog::TransmitProgressDialog()
    : mTotal(1)
{
    CtrlLayout(*this);
    mProgress.Percent(true);
}
void TransmitProgressDialog::SetTotal(size_t total)
{
    mTotal = total;
}

void TransmitProgressDialog::Update(size_t count, double tx_rate)
{
    mProgress.Set(count, mTotal);
    // tx rate
    char buff[256]; // 256 is large enough for double.2
    sprintf(buff, "%.2lf", tx_rate / 1024);
    mTxRate.SetText((std::string(t_("Tx Rate:")) + buff + "KiB/s").c_str());
    // left time
    if (tx_rate == 0.) tx_rate = 1.;
    double time = (mTotal - count) / tx_rate;
    // HH:mm:ss
    size_t hours = (size_t)time / 3600;
    size_t minutes = (size_t)(time - hours*3600) / 60;
    size_t seconds = (size_t)(time - hours*3600 - minutes*60);
    sprintf(buff, "%zu:%02zu:%02zu", hours, minutes, seconds);
    mTime.SetText((std::string(t_("Left Time:")) + buff).c_str());
}