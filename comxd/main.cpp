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
	//
	struct TabCloseBtn : public Button {
	public:
	    TabCloseBtn(Ctrl* conn, TabCtrl* tab_bar)
	        : mConn(conn)
	        , mTabbar(tab_bar)
	    {
	        *this << [&]() { //<! on clicked
	            // the callback will execute in the main thread (i.e GUI-thread), so
	            // we should save the conn and tab_bar in the instance of TabCloseBtn.
	            Upp::PostCallback([=]() {
	                delete this; // delete myself.
	            });
	        };
	    }
	    virtual ~TabCloseBtn()
	    {
	        mTabbar->Remove(*mConn);
	        delete mConn;
	    }
	    //
    private:
        Ctrl* mConn;
        TabCtrl* mTabbar;
	};
	// create a new connection
	void NewConn()
	{
	    SerialDevsDialog d;
		auto conn = d.RequestConn();
		if (conn) {
		    auto btn_close = new TabCloseBtn(conn, &mDevsTab);
		    btn_close->SetImage(comxd::close_little());
		    btn_close->SetRect(0, 0, 16, 16);
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
