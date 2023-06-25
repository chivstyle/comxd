/*!
// (c) 2022 chiv
//
*/
#include "resource.h"
#include "AboutDialog.h"

using namespace Upp;

AboutDialog::AboutDialog()
{
	Title(Upp::GetAppName());
	
	mOk.Ok() << Acceptor(IDOK);
	
	this->Sizeable(true);
	
	CtrlLayout(*this);
}

void AboutDialog::SetQTF(const char* qtf)
{
	mText.SetQTF(qtf);
}
