/*!
// (c) 2022 chiv
//
*/
#ifndef _comxd_AboutDialog_h_
#define _comxd_AboutDialog_h_

#include <CtrlLib/CtrlLib.h>

class AboutDialog : public WithAboutDialog<Upp::TopWindow> {
public:
	typedef AboutDialog CLASSNAME;
	AboutDialog();
	//
	void SetQTF(const char* qtf);
};

#endif
