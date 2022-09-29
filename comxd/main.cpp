//
// (C) 2020 chiv
//
#include "resource.h"
#include "CodecTool.h"
#include "ConnCreateFactory.h"

// main window
class MainWindow : public WithMainWindow<TopWindow> {
    typedef MainWindow CLASSNAME;
public:
    MainWindow()
    {
        MakeConfigDirectory();
        //
        this->Title("comxd");
        //
        this->CenterScreen();
        this->Sizeable();
        this->Icon(comxd::app_icon()).MaximizeBox().MinimizeBox();
        // status bar
        InitStatusbar();
        // welcome to comxd.
        mAbout.SetQTF(Upp::GetTopicLNG("comxd/comxd/welcome"));
        //
        mToolbar.Set(THISBACK(MainToolbar));
        // frame
        AddFrame(mToolbar);
        AddFrame(mStatusbar);
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
            // never reach here
            delete mDevsTab.GetItem(0).GetCtrl();
        }
    }
    //
    void MakeConfigDirectory()
    {
        auto conf = Upp::GetHomeDirectory() + "/.comxd";
        if (!Upp::DirectoryExists(conf)) {
            Upp::DirectoryCreate(conf);
        }
    }
    //
    struct TabCloseBtn : public Button {
    public:
        TabCloseBtn(Ctrl* conn, TabCtrl* tab_bar, Ctrl* about)
            : mConn(conn)
            , mTabbar(tab_bar)
            , mAbout(about)
        {
            *this << [&]() { //<! on clicked
                // the callback will execute in the main thread (i.e GUI-thread), so
                // we should save the conn and tab_bar in the instance of TabCloseBtn.
                Upp::PostCallback([=]() {
                    // before release the conn, we must stop the Io firstly
                    static_cast<SerialConn*>(mConn)->GetIo()->Stop();
                    mTabbar->Remove(*mConn);
                    delete mConn;
                    if (mTabbar->GetCount() == 0) {
                        mAbout->Show();
                    }
                    delete this; // delete myself.
                });
            };
        }
        //
    private:
        Ctrl* mConn;
        TabCtrl* mTabbar;
        Ctrl* mAbout;
    };
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
        auto& conns = ConnCreateFactory::Inst()->GetSupportedConnIntroductions();
        for (auto it = conns.begin(); it != conns.end(); ++it) {
            bar.Add(it->second.Name, it->second.Icon(), [=]() {
                // If we run codes here diretly, the program will crash, so we
                // put these codes to PostCallback
                PostCallback([=]() {
                    auto conn = it->second.Create();
                    if (conn) {
                        conn->WhenWarning = [=](String warning) {
                            mStatusbar.Set(1, warning, 500);
                        };
                        auto btn_close = new TabCloseBtn(conn, &mDevsTab, &mAbout);
                        btn_close->SetImage(comxd::close_little());
                        btn_close->SetRect(0, 0, 16, 16);
                        mDevsTab.Add(conn->SizePos(), it->second.Icon(), conn->ConnName()).SetCtrl(btn_close);
#if 0
                        conn->WhenTitle = [=](String title) {
                            auto item = FindConnItem(conn);
                            if (item)
                                item->Text(title);
                        };
#endif
                        // set current to conn
                        mDevsTab.Set(*conn);
                        //
                        mAbout.Hide();
                    }
                });
            }).Tip(it->second.Desc);
        }
        //
        bar.ToolSeparator();
        //
        int icnt = bar.GetChildCount();
        OndemandToolbar(bar);
        if (icnt != bar.GetChildCount()) {
            bar.Separator();
        }
        bar.Add(t_("Codec Tool"), comxd::codec_tool(), [=]() { CodecTool d; d.Run(true); });
        //
        bar.ToolGapRight();
        bar.Add(t_("About"), comxd::about(), [=]() { PromptOK(Upp::GetTopicLNG("comxd/comxd/about")); });
    }
    
    TabCtrl::Item* FindConnItem(const SerialConn* conn_)
    {
        int cnt = mDevsTab.GetCount();
        for (int i = 0; i < cnt; ++i) {
            TabCtrl::Item& item = mDevsTab.GetItem(i);
            auto conn = dynamic_cast<SerialConn*>(item.GetSlave());
            if (conn == conn_)
                return &item;
        }
        return nullptr;
    }

    void OndemandToolbar(Bar& bar)
    {
        if (mDevsTab.GetCount()) {
            auto conn = dynamic_cast<SerialConn*>(mDevsTab.GetItem(mDevsTab.Get()).GetSlave());
            if (conn) {
                bar.Add(t_("Reconnect"), comxd::reconnect(), [=]() {
                    this->Disable();
                    conn->Stop();
                    conn->GetIo()->Stop();
                    if (!conn->GetIo()->Start()) {
                        String text = t_("Can't start the I/O deivce:");
                        text += conn->GetIo()->DeviceName();
                        PromptOK(Upp::DeQtf(text));
                    } else {
                        conn->Start();
                    }
                    this->Enable();
	            }).Help(t_("Reconnect to the I/O device"));
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
