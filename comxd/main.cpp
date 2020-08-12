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
		InitStatusbar();
		//
		AddFrame(mToolbar);
		AddFrame(mStatusbar);
		mToolbar.Set(THISBACK(MainToolbar));
		mDevsTab.WhenSet = THISBACK(OnDevsTabSet);
		//
		Upp::CtrlLayout(*this);
	}
	~MainWindow()
	{
	    for (int i = 0; i < mDevsTab.GetCount(); ++i) {
	        delete mDevsTab.GetItem(i).GetCtrl();
	    }
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
		    mDevsTab.Set(*conn);
		}
	}
	// change current settings
	void ChangeSettings()
	{
	    if (mDevsTab.GetCount()) {
            auto conn = dynamic_cast<SerialConn*>(mDevsTab.GetItem(mDevsTab.Get()).GetSlave());
            if (conn) {
                SerialDevsDialog d;
                d.ChangeSettings(conn->GetSerial());
            }
	    }
	}
	//
	void OnDevsTabSet()
	{
	    mToolbar.Set(THISBACK(MainToolbar));
	    // focus it
	    if (mDevsTab.GetCount()) {
            auto conn = dynamic_cast<SerialConn*>(mDevsTab.GetItem(mDevsTab.Get()).GetSlave());
            if (conn) {
                conn->SetFocus();
            }
	    }
	}
	//
	bool Accept()
	{
	    if (mDevsTab.GetCount()) {
            int ret = Upp::PromptYesNo(t_("Do you want to exit really?"));
            // ret , 1 - yes, 0 - None, -1 - cancel
            return ret == 1;
	    } else return true;
	}
	
protected:
	// widgets
	ToolBar mToolbar;
	StatusBar mStatusbar;
	// Add a toolbar for window
	void MainToolbar(Bar& bar)
	{
		bar.Add(t_("New Connection"), comxd::new_conn(), THISBACK(NewConn));
		bar.Add(t_("Settings"), comxd::settings(), THISBACK(ChangeSettings));
		bar.ToolSeparator();
		//
		OndemandToolbar(bar);
		//
		bar.ToolGapRight();
	    bar.Add(t_("About"), comxd::about(), [=](){
	        PromptOK(Upp::GetTopicLNG("comxd/comxd/about"));
	    });
	}
	void OndemandToolbar(Bar& bar)
	{
	    if (mDevsTab.GetCount()) {
	        auto conn = dynamic_cast<SerialConn*>(mDevsTab.GetItem(mDevsTab.Get()).GetSlave());
	        if (conn) {
	            auto actions = conn->GetActions();
	            for (auto it = actions.begin(); it != actions.end(); ++it) {
	                bar.Add(it->Text, it->Icon, it->Func).Help(it->Help);
	            }
	        }
	    }
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
