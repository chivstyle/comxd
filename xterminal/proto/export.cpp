/*!
// (c) 2022 chiv
//
*/
#include "terminal_rc.h"
#include "TransmitProgressDialog.h"
#include "export.h"

ProgressDialog::ProgressDialog()
	: mImpl(new TransmitProgressDialog)
{
	mImpl->WhenClose = [=]() { WhenClose(); };
}
ProgressDialog::~ProgressDialog()
{
	delete mImpl;
}
void ProgressDialog::SetTotal(int64 total)
{
	mImpl->SetTotal(total);
}
void ProgressDialog::Update(int64 bytes, double rate)
{
	mImpl->Update(bytes, rate);
}
void ProgressDialog::Close()
{
	mImpl->Close();
}
int ProgressDialog::Run(bool modal)
{
	return mImpl->Run(modal);
}
void ProgressDialog::Title(const Upp::String& title)
{
	mImpl->Title(title);
}