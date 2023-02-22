//
// (c) 2021 chiv
//
#include "terminal_rc.h"
#include "TransmitProgressDialog.h"

using namespace Upp;

TransmitProgressDialog::TransmitProgressDialog()
    : mTotal(1)
{
    CtrlLayout(*this);
    mProgress.Percent(true);
}
void TransmitProgressDialog::SetTotal(int64 total)
{
    mTotal = total;
}

void TransmitProgressDialog::Update(int64 count, double tx_rate)
{
	int curr = int(count * 1000 / mTotal);
	mProgress.Set(curr, 1000);
	// human readable
	std::string unit;
	double rate;
	if (tx_rate < 1024.) {
		unit = "B/s";
	} else if (tx_rate < 1024*1024.) {
		rate = tx_rate / 1024;
		unit = "KiB/s";
	} else if (tx_rate < 1024*1024*1024.) {
		rate = tx_rate / 1024 / 1024;
		unit = "MiB/s";
	} else {
		rate = tx_rate / 1024 / 1024 / 1024;
		unit = "GiB/s";
	}
    // tx rate
    char buff[256]; // 256 is large enough for double.2
    sprintf(buff, "%.2lf", rate);
    mTxRate.SetText((std::string(t_("Tx Rate:")) + buff + unit).c_str());
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
