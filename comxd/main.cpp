//
// (C) 2020 chiv
//
#include "resource.h"
// dialogs or forms
#include "ioimpls/SSHDevsDialog.h"
#include "ioimpls/SerialDevsDialog.h"
// main window
class MainWindow : public WithMainWindow<TopWindow> {
    typedef MainWindow CLASSNAME;

public:
    MainWindow()
    {
        this->Title("comxd");
        //
        this->CenterScreen();
        this->Sizeable();
        this->Icon(comxd::app_icon()).MaximizeBox().MinimizeBox();
        // status bar
        InitStatusbar();
        // welcome to comxd.
        mAbout.SetQTF(Upp::GetTopicLNG("comxd/comxd/welcome"));
        // frame
        AddFrame(mToolbar);
        AddFrame(mStatusbar);
        // Toolbar
        mToolbar.Set(THISBACK(MainToolbar));
        // conn tabs.
        mDevsTab.WhenSet = THISBACK(OnDevsTabSet);
        mDevsTab.Add(mAbout);
        // layout
        Upp::CtrlLayout(*this);
    }
    ~MainWindow()
    {
        int cnt = mDevsTab.GetCount();
        while (cnt--) {
            delete mDevsTab.GetItem(0).GetCtrl();
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
                    // before release the conn, we must stop the Io firstly
                    static_cast<SerialConn*>(mConn)->GetIo()->Stop();
                    mTabbar->Remove(*mConn);
                    delete mConn;
                    delete this; // delete myself.
                });
            };
        }
        //
    private:
        Ctrl* mConn;
        TabCtrl* mTabbar;
    };
    // create a new connection
    void NewSerialConn()
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
    void NewSSHConn()
    {
        SSHDevsDialog d;
        auto conn = d.RequestConn();
        if (conn) {
            auto btn_close = new TabCloseBtn(conn, &mDevsTab);
            btn_close->SetImage(comxd::close_little());
            btn_close->SetRect(0, 0, 16, 16);
            mDevsTab.Add(conn->SizePos(), conn->ConnName()).SetCtrl(btn_close);
            mDevsTab.Set(*conn);
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
            if (ret == 1) {
                int cnt = mDevsTab.GetCount();
                for (int i = 0; i < cnt; ++i) {
                    TabCtrl::Item& item = mDevsTab.GetItem(i);
                    auto conn = dynamic_cast<SerialConn*>(item.GetSlave());
                    if (conn) {
                        // before release the conn, we must stop the Io firstly
                        conn->GetIo()->Stop();
                    }
                    delete item.GetCtrl();
                    delete item.GetSlave();
                }
            }
            return ret == 1;
        } else
            return true;
    }

protected:
    // widgets
    ToolBar mToolbar;
    StatusBar mStatusbar;
    RichTextCtrl mAbout;
    // Add a toolbar for window
    void MainToolbar(Bar& bar)
    {
        bar.Add(t_("New Serial"), comxd::new_serial(), THISBACK(NewSerialConn));
        bar.Add(t_("New SSH"), comxd::new_ssh(), THISBACK(NewSSHConn));
        bar.ToolSeparator();
        //
        OndemandToolbar(bar);
        //
        bar.ToolGapRight();
        bar.Add(t_("About"), comxd::about(), [=]() { PromptOK(Upp::GetTopicLNG("comxd/comxd/about")); });
    }

    void OndemandToolbar(Bar& bar)
    {
        if (mDevsTab.GetCount()) {
            auto conn = dynamic_cast<SerialConn*>(mDevsTab.GetItem(mDevsTab.Get()).GetSlave());
            if (conn) {
                // Actions of serial device
                conn->GetIo()->WhenUsrBar(bar);
                conn->WhenUsrBar(bar);
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
    MainWindow win;
    win.SetRect(0, 0, 800, 600);
    win.Run();
}
