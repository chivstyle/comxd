//
// (c) 2020 chiv
//
#include "terminal_rc.h"
#include "SerialConnXterm.h"
#include "ConnFactory.h"
// dialogs
#include "TextCodecsDialog.h"
#include "VTOptionsDialog.h"

using namespace Upp;

REGISTER_CONN_INSTANCE("Xterm powered by upp-components", "xterm", SerialConnXterm);

SerialConnXterm::SerialConnXterm(std::shared_ptr<SerialIo> serial)
	: SerialConn(serial)
	, mIoThrShouldStop(false)
{
	Sizeable().Add(mTerm.SizePos());
	mTerm.WhenOutput = [=](String s) {
		std::string out = GetCodec()->TranscodeFromUTF8((unsigned char*)s.Begin(), s.GetLength());
		GetIo()->Write(out);
	};
	mTerm.ShowScrollBar();
	//
	this->PostCallback([=]() {
		mTerm.SetFocus();
	});
	//
	InstallActions();
}

SerialConnXterm::~SerialConnXterm()
{
	mIoThrShouldStop = true;
	if (mIoThr.joinable()) {
		mIoThr.join();
	}
}

void SerialConnXterm::InstallActions()
{
	mUsrActions.emplace_back(terminal::text_codec(),
        t_("Text Codec"), t_("Select a text codec"), [=]() {
            TextCodecsDialog d(GetCodec()->GetName().c_str());
            int ret = d.Run();
            if (ret == IDOK) {
                std::lock_guard<std::mutex> _(mRxLock);
                // rx thread is using the codec, so we should wait lock then update it.
                this->SetCodec(d.GetCodecName());
            }
        });
    mUsrActions.emplace_back(terminal::clear_buffer(),
        t_("Clear Buffer"), t_("Clear the line buffers"), [=]() {
            mTerm.ClearHistory();
        });
    mUsrActions.emplace_back(terminal::vt_options(),
        t_("VT options"), t_("Virtual terminal options"), [=]() {
            VTOptionsDialog::Options options;
            options.Font = mTerm.GetFont();
            options.LinesBufferSize = mTerm.GetHistorySize();
            options.PaperColor = mTerm.GetColor(Terminal::COLOR_PAPER);
            options.FontColor = mTerm.GetColor(Terminal::COLOR_INK);
            VTOptionsDialog opt;
            opt.SetOptions(options);
            int ret = opt.Run();
            if (ret == IDOK) {
                options = opt.GetOptions();
                mTerm.SetFont(options.Font);
                mTerm.SetHistorySize(options.LinesBufferSize);
                mTerm.SetColor(Terminal::COLOR_PAPER, options.PaperColor);
                mTerm.SetColor(Terminal::COLOR_INK, options.FontColor);
                mTerm.Refresh();
                //
                DoLayout();
            }
        });
}

void SerialConnXterm::DoLayout()
{
	Size csz = mTerm.GetPageSize();
	if (csz.cx > 1 && csz.cy > 1) {
	    WhenSizeChanged(csz);
	}
}

void SerialConnXterm::Layout()
{
	DoLayout();
}

bool SerialConnXterm::Start()
{
	mIoThr = std::thread([=]() {
		while (!mIoThrShouldStop) {
			int sz = GetIo()->Available();
	        if (sz < 0) {
	            PostCallback([=]() {
	                PromptOK(DeQtf(t_("Fatal error of I/O")));
	            });
	            break;
	        } else if (sz == 0) {
	            std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
	            continue;
	        }
			std::vector<unsigned char> buff = GetIo()->ReadRaw(sz);
			mRxLock.lock();
			mRxBuffer.insert(mRxBuffer.end(), buff.begin(), buff.end());
			PostCallback([=] {
				mRxLock.lock();
				std::string s = GetCodec()->TranscodeToUTF8(mRxBuffer.data(), mRxBuffer.size());
				mRxBuffer.clear();
				mRxLock.unlock();
				//
				mTerm.Write(s.c_str(), (int)s.length()); // UTF-8
			});
			mRxLock.unlock();
		}
	});
	return true;
}
