//
// (C) 2020 chiv
//
#include "resource.h"
// dialogs or forms
#include "SerialDevsDialog.h"
// main window
class MainWindow : public WithMainWindow<TopWindow> {
	typedef MainWindow CLASSNAME;
public:
	MainWindow()
	{
	    this->Title("comxd");
	    //
		this->SetRect(0, 0, 800, 600);
		this->CenterScreen();
		this->Sizeable();
		this->Icon(comxd::app_icon()).MaximizeBox().MinimizeBox();
		// initialize widgets
		InitToolbar();
		InitStatusbar();
		//
		AddFrame(mToolbar);
		AddFrame(mStatusbar);
		//
		Upp::CtrlLayout(*this);
	}
	
	// create a new connection
	void NewConn()
	{
	    SerialDevsDialog d;
		auto conn = d.RequestConn();
		if (conn) {
		    Button* btn_close = new Button();
		    btn_close->SetImage(comxd::close_little());
		    btn_close->SetRect(0, 0, 20, 20);
		    *btn_close << [&]() {
		        mDevsTab.Remove(*conn);
		        delete conn;
		        delete btn_close;
		    };
		    mDevsTab.Add(conn->SizePos(), conn->ConnName()).SetCtrl(btn_close);
		}
	}
	// change current settings
	void ChangeSettings()
	{
	    auto conn = dynamic_cast<SerialConn*>(mDevsTab.GetItem(mDevsTab.Get()).GetSlave());
	    if (conn) {
            SerialDevsDialog d;
            d.ChangeSettings(conn->GetSerial());
	    }
	}
	//
	bool Accept()
	{
	    int ret = Upp::PromptYesNo(t_("Do you want to exit really?"));
	    // ret , 1 - yes, 0 - None, -1 - cancel
	    return ret == 1;
	}
	
protected:
	// widgets
	ToolBar mToolbar;
	StatusBar mStatusbar;
	// Add a toolbar for window
	void InitToolbar()
	{
		mToolbar.Add(t_("New Connection"), comxd::new_conn(), THISBACK(NewConn));
		mToolbar.Add(t_("Settings"), comxd::settings(), THISBACK(ChangeSettings));
	}
	void InitStatusbar()
	{
		mStatusbar.Set(0, t_("Welcom to comxd!"), 100);
	}
};

GUI_APP_MAIN
{
	Upp::SetLanguage(GetSystemLNG());
	//
	MainWindow().Run();
}
