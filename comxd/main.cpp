//
// (C) 2020 chiv
//
#ifdef _MSC_VER
#include <winsock2.h>
#include <MinHook.h>
#include <Windows.h>
#include <minidumpapiset.h>
#pragma comment(lib, "dbghelp.lib")
#endif
#include "resource.h"
#include "ConnCreateFactory.h"
//
#ifdef _MSC_VER
static
LONG WINAPI ApplicationCrashHandler(EXCEPTION_POINTERS* exception_pointers)
{
    std::wstring fname_ = (Upp::GetTempDirectory() + "\\comxd.dump").ToWString().ToStd();
    const WCHAR* fname = fname_.c_str();
    HANDLE file = CreateFileW(fname, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        return EXCEPTION_CONTINUE_SEARCH;
    } else {
        MINIDUMP_EXCEPTION_INFORMATION mei;
        mei.ExceptionPointers = exception_pointers;
        mei.ThreadId = GetCurrentThreadId();
        mei.ClientPointers = TRUE;
        BOOL ret = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpNormal,
            &mei, NULL, NULL);
        MessageBoxW(NULL, fname, L"Exception", MB_OK);
        return ret == TRUE ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
    }
}

typedef LPTOP_LEVEL_EXCEPTION_FILTER (*FnSetUnhandledExceptionFilter)(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);
static FnSetUnhandledExceptionFilter NtSetUnhandledExceptionFilter = NULL;

LPTOP_LEVEL_EXCEPTION_FILTER MySetUnhandledExceptionFilter(
  LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter
)
{
    (void)lpTopLevelExceptionFilter;
    // DO NOTHING HERE
    return NULL;
}

static void my_invalid_parameter_handler(const wchar_t* expression, const wchar_t* function, const wchar_t* file,
    unsigned int line, uintptr_t reserved)
{
    (void)function;
    (void)file;
    (void)reserved;
    throw expression;
}
static void my_purecall_handler()
{
    throw "purecall";
}

static void CrashReport_Initialize()
{
#ifndef _DEBUG
    _set_invalid_parameter_handler(my_invalid_parameter_handler);
    _set_purecall_handler(my_purecall_handler);
#endif
    SetUnhandledExceptionFilter(ApplicationCrashHandler);
    MH_STATUS sr = MH_Initialize();
    if (sr == MH_OK) {
        NtSetUnhandledExceptionFilter = (FnSetUnhandledExceptionFilter)GetProcAddress(GetModuleHandleA("kernel32.dll"),
            "SetUnhandledExceptionFilter");
        if (NtSetUnhandledExceptionFilter) {
            MH_CreateHook(NtSetUnhandledExceptionFilter, MySetUnhandledExceptionFilter, NULL);
            MH_EnableHook(NtSetUnhandledExceptionFilter);
        }
    } else {
        MessageBoxW(NULL, L"We can not initialize crash report on your system.", L"Warning", MB_OK);
    }
}
static void CrashReport_Shutdown()
{
    if (NtSetUnhandledExceptionFilter) {
        MH_RemoveHook(NtSetUnhandledExceptionFilter);
        //
        MH_Uninitialize();
    }
}

#endif
// main window
class MainWindow : public WithMainWindow<TopWindow> {
    typedef MainWindow CLASSNAME;

public:
    MainWindow()
    {
#if _MSC_VER
        CrashReport_Initialize();
#endif
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
            // never reach here
            delete mDevsTab.GetItem(0).GetCtrl();
        }
#if _MSC_VER
        CrashReport_Shutdown();
#endif
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
            bar.Add(it->second.Name, it->second.Icon, [=]() {
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
                        mDevsTab.Add(conn->SizePos(), conn->ConnName()).SetCtrl(btn_close);
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
        OndemandToolbar(bar);
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
