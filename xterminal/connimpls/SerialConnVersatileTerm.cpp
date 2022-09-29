/*!
// (c) 2022 chiv
//
*/
#include "terminal_rc.h"
#include "SerialConnVersatileTerm.h"
#include "TextCodecsDialog.h"
#include "ConnFactory.h"
#include "VTOptionsDialog.h"
#include "ProtoFactory.h"
#include "ColorTable.h"

using namespace xvt;

REGISTER_CONN_INSTANCE("versatile xterm", "xterm-256color", SerialConnVersatileTerm);

SerialConnVersatileTerm::SerialConnVersatileTerm(std::shared_ptr<SerialIo> io)
	: SerialConn(io)
	, mRxShouldStop(false)
{
	this->SetConnDescription("Versatile Xterm, powered by TerminalCtrl.");
	this->Add(mVt.SizePos());
	//
	mVt.WhenOutput = [=](String data) { GetIo()->Write(data.ToStd()); };
	mVt.WhenResize = [=]() { this->WhenSizeChanged(mVt.GetPageSize()); };
	mVt.InlineImages().Hyperlinks().WindowOps();
	mVt.ShowScrollBar(true);
	// set the color table
	VTColorTable tbl;
	mVt.SetColor(TerminalCtrl::COLOR_BLACK, tbl.GetColor(VTColorTable::kColorId_Black));
	mVt.SetColor(TerminalCtrl::COLOR_BLUE, tbl.GetColor(VTColorTable::kColorId_Blue));
	mVt.SetColor(TerminalCtrl::COLOR_CYAN, tbl.GetColor(VTColorTable::kColorId_Cyan));
	mVt.SetColor(TerminalCtrl::COLOR_GREEN, tbl.GetColor(VTColorTable::kColorId_Green));
	mVt.SetColor(TerminalCtrl::COLOR_INK, tbl.GetColor(VTColorTable::kColorId_Texts));
	mVt.SetColor(TerminalCtrl::COLOR_INK_SELECTED, tbl.GetColor(VTColorTable::kColorId_Paper));
	//mVt.SetColor(TerminalCtrl::COLOR_LTBLACK, tbl.GetColor(VTColorTable::kColorId_xx));
	//mVt.SetColor(TerminalCtrl::COLOR_LTBLUE, tbl.GetColor(VTColorTable::kColorId_Black));
	//mVt.SetColor(TerminalCtrl::COLOR_LTMAGENTA, tbl.GetColor(VTColorTable::kColorId_Black));
	//mVt.SetColor(TerminalCtrl::COLOR_LTRED, tbl.GetColor(VTColorTable::kColorId_Black));
	//mVt.SetColor(TerminalCtrl::COLOR_LTWHITE, tbl.GetColor(VTColorTable::kColorId_Black));
	//mVt.SetColor(TerminalCtrl::COLOR_LTYELLOW, tbl.GetColor(VTColorTable::kColorId_Black));
	mVt.SetColor(TerminalCtrl::COLOR_MAGENTA, tbl.GetColor(VTColorTable::kColorId_Magenta));
	mVt.SetColor(TerminalCtrl::COLOR_PAPER, tbl.GetColor(VTColorTable::kColorId_Paper));
	mVt.SetColor(TerminalCtrl::COLOR_PAPER_SELECTED, tbl.GetColor(VTColorTable::kColorId_Texts));
	mVt.SetColor(TerminalCtrl::COLOR_RED, tbl.GetColor(VTColorTable::kColorId_Red));
	mVt.SetColor(TerminalCtrl::COLOR_WHITE, tbl.GetColor(VTColorTable::kColorId_White));
	mVt.SetColor(TerminalCtrl::COLOR_YELLOW, tbl.GetColor(VTColorTable::kColorId_Yellow));
	//
	InstallUserActions();
}

SerialConnVersatileTerm::~SerialConnVersatileTerm()
{
	Stop();
}

void SerialConnVersatileTerm::GotFocus()
{
	mVt.SetFocus();
}

void SerialConnVersatileTerm::ShowVTOptions()
{
    VTOptionsDialog::Options options;
    options.Font = mVt.GetFont();
    options.LinesBufferSize = mVt.GetHistorySize();
    options.PaperColor = mVt.GetColor(TerminalCtrl::COLOR_PAPER);
    options.TextsColor = mVt.GetColor(TerminalCtrl::COLOR_INK);
    VTOptionsDialog opt;
    opt.SetOptions(options);
    int ret = opt.Run();
    if (ret == IDOK) {
        options = opt.GetOptions();
        mVt.SetHistorySize(options.LinesBufferSize);
        mVt.SetFont(options.Font).SetColor(TerminalCtrl::COLOR_PAPER, options.PaperColor)
            .SetColor(TerminalCtrl::COLOR_INK, options.TextsColor);
    }
}

void SerialConnVersatileTerm::InstallUserActions()
{
	WhenUsrBar = [=](Bar& bar) {
        bar.Add(t_("Text Codec"), terminal::text_codec(), [=]() {
           TextCodecsDialog d(GetCodec()->GetName().c_str());
           int ret = d.Run();
           if (ret == IDOK) {
               this->SetCodec(d.GetCodecName());
               Refresh();
           }
        }).Help(t_("Select a text codec"));
        bar.Add(t_("Clear Buffer"), terminal::clear_buffer(), [=]() { mVt.HardReset(); })
            .Help(t_("Clear the line buffers"));
        bar.Add(t_("VT Options"), terminal::vt_options(), [=]() { ShowVTOptions(); })
            .Help(t_("Virtual terminal options"));
        bar.Add(t_("Information"), terminal::info(), [=]() {
            std::string info(t_("Basic information")); info += "&";
            Size csz = mVt.GetPageSize();
            info += "  [* "; info += t_("Size "); info += "]-|" + std::to_string(csz.cx) + "x" + std::to_string(csz.cy) + "&";
            info += "  [* "; info += t_("Desc "); info += "]-|" + DeQtf(GetConnDescription()) + "&";
            info += "  [* "; info += t_("Io   "); info += "]-|" + DeQtf(GetIo()->DeviceName().c_str()) + "&";
            info += "  [* "; info += t_("Codec"); info += "]-|" + DeQtf(GetCodec()->GetName().c_str()) + "&";
            Upp::PromptOK(info.c_str());
        }).Help(t_("Show basic vt information"));
    };
    // Context menu bar
    mVt.WhenBar = [=](Bar& bar) {
        bool has_sel = mVt.IsSelection();
        bar.Add(has_sel, t_("Copy"), [=] {
           mVt.Copy();
        }).Image(CtrlImg::copy()).Key(K_CTRL | K_SHIFT | K_C);
        bar.Add(has_sel, t_("Copy"), [=] {
           mVt.Copy();
        }).Image(CtrlImg::copy()).Key(K_CTRL | K_INSERT);
        String text = ReadClipboardUnicodeText().ToString();
        bar.Add(text.GetCount() > 0, t_("Paste"), [=] { mVt.Paste(); })
            .Image(CtrlImg::paste())
            .Key(K_CTRL | K_SHIFT | K_V);
        bar.Add(text.GetCount() > 0, t_("Paste"), [=] { mVt.Paste(); })
            .Image(CtrlImg::paste())
            .Key(K_SHIFT | K_INSERT);
        bar.Add(true, t_("Select all"), [=]() { mVt.SelectAll(true); })
            .Key(K_CTRL | K_SHIFT | K_A);
        //--------------------------------------------------------------------------------------
        bar.Sub(t_("Transmit File(s)"), [=](Bar& sub) {
            auto proto_names = ProtoFactory::Inst()->GetSupportedProtoNames();
            for (size_t k = 0; k < proto_names.size(); ++k) {
                sub.Add(proto_names[k], [=]() {
                    auto proto = ProtoFactory::Inst()->CreateInst(proto_names[k], this);
                    proto->TransmitFile();
                    delete proto;
                });
            }
        });
        //--------------------------------------------------------------------------------------
    };
}

// receiver
void SerialConnVersatileTerm::RxProc()
{
    std::string raw;
    while (!mRxShouldStop) {
        int sz = GetIo()->Available();
        if (sz < 0) {
            Upp::PostCallback([=]() { PromptOK(DeQtf(this->ConnName() + ":" + t_("I/O device was disconnected"))); });
            break;
        } else if (sz == 0) {
            std::this_thread::sleep_for(std::chrono::duration<double>(0.001));
            continue;
        }
        // read raw, not string, so, we could read NUL from the device.
        std::vector<byte> buff = GetIo()->ReadRaw(sz);
        for (size_t k = 0; k < buff.size(); ++k) {
            if (buff[k] != 0) // IGNORE NULL
                raw.push_back(buff[k]);
        }
        // Transcode to UTF8
        auto s = this->GetCodec()->TranscodeToUTF8(buff.data(), buff.size());
        raw.clear();
        //
        Upp::EnterGuiMutex();
        mVt.WriteUtf8(s);
        Upp::LeaveGuiMutex();
    }
}

bool SerialConnVersatileTerm::Start()
{
	mRxShouldStop = false;
    mRxThr = std::thread([=]() { RxProc(); });
    Upp::PostCallback([=] { Layout(); mVt.WhenResize(); });
    //
    return true;
}
void SerialConnVersatileTerm::Stop()
{
	mRxShouldStop = true;
    if (mRxThr.joinable()) {
        mRxThr.join();
    }
}
