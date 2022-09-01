/*!
// (c) 2022 chiv
//
*/
#include "resource.h"
#include "SSHPort.h"
#include "proto/export.h"
#include <thread>

using namespace Upp;

static inline String GetBasename(const String& pathname)
{
	String p = pathname;
	p.Replace("\\", "/");
	int idx = p.Find("/");
	if (idx < 0) return pathname;
	else {
		int next = p.Find("/", idx+1);
		while (next >= 0) {
			idx = next;
			next = p.Find("/", idx+1);
		}
		return p.Mid(idx+1);
	}
}

void SSHPort::Upload()
{
	FileSel fs;
    if (fs.AllFilesType().ExecuteOpen()) {
        auto filename = fs.Get();
        // please input the destination
        EditString es;
        Button btn_ok;
        btn_ok.SetLabel(t_("Ok"));
        TopWindow d;
        d.Title(t_("Remote File"));
        d.SetRect(0, 0, Zx(324), Zy(36));
        d.Add(es.HSizePosZ(12, 76).TopPosZ(8, 19));
        d.Add(btn_ok.RightPosZ(12, 56).TopPosZ(8, 20));
        d.Acceptor(btn_ok, IDOK);
        d.WhenClose = [&]() {
            d.RejectBreak(IDCANCEL);
        };
        int ret = d.Run(true);
        if (ret == IDOK) {
            double t_rate = 0;
            int64 t_count = 0, t_total = 0;
            String remote = ~es;
            //
            FileIn fin(filename);
            Scp scp = mSession->CreateScp();
            // try to fix, why don't we transmit file by ftp ? It's slower than scp heavily.
            SFtp ftp = mSession->CreateSFtp();
            SFtp::DirEntry info = ftp.GetInfo(remote);
            if (info.IsDirectory()) {
                remote += "/" + GetBasename(filename);
            }
            static volatile bool should_stop = false;
            //
            std::mutex lock;
            ProgressDialog bar;
            bar.WhenClose = [&]() { should_stop = true; };
            auto t1 = std::chrono::high_resolution_clock::now();
            scp.WhenProgress = [&](int64 count, int64 total) {
                double ts = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t1).count();
                std::lock_guard<std::mutex> _(lock);
                t_rate = count / ts;
                t_count = count;
                t_total = total;
                PostCallback([&]() { std::lock_guard<std::mutex> _(lock); bar.SetTotal(t_total); bar.Update(t_count, t_rate); });
                return count >= total || should_stop;
            };
            bool success = false;
            std::thread job([&]() {
                success = scp.SaveFile(remote, fin);
                PostCallback([&]() { bar.Close(); });
            });
            bar.Run(true);
            job.join();
            if (!should_stop) {
	            PostCallback([=]() {
	                PromptOK(success ? Upp::DeQtf(t_("Transmition was failed!")) : Upp::DeQtf(t_("Transmition was completed!")));
	            });
            }
        }
    }
}

void SSHPort::Download()
{
	FileSel fs;
    if (fs.ExecuteSelectDir()) {
        auto filename = fs.Get();
        // please input the destination
        EditString es;
        TopWindow d;
        Button btn_ok;
        btn_ok.SetLabel(t_("Ok"));
        d.Title(t_("Remote File"));
        d.SetRect(0, 0, Zx(324), Zy(36));
        d.Add(es.HSizePosZ(12, 76).TopPosZ(8, 19));
        d.Add(btn_ok.RightPosZ(12, 56).TopPosZ(8, 20));
        d.Acceptor(btn_ok, IDOK);
        d.WhenClose = [&]() {
            d.RejectBreak(IDCANCEL);
        };
        int ret = d.Run(true);
        if (ret == IDOK) {
            double r_rate = 0;
            int64  r_total = 0, r_count = 0;
            String remote = ~es;
            //
            Scp scp = mSession->CreateScp();
            // try to fix
            SFtp ftp = mSession->CreateSFtp();
            SFtp::DirEntry info = ftp.GetInfo(remote);
            if (!info.IsFile()) {
                PromptOK(Upp::DeQtf("The remote file is not a normal file!"));
                return;
            }
            // check
            static volatile bool should_stop = false;
            FileOut fout(filename);
            ProgressDialog bar;
            bar.WhenClose = [&]() { should_stop = true; };
            std::mutex lock;
            auto t1 = std::chrono::high_resolution_clock::now();
            scp.WhenProgress = [&](int64 count, int64 total) {
                double ts = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t1).count();
                std::lock_guard<std::mutex> _(lock);
                r_rate = count / ts;
                r_total = total;
                r_count = count;
                PostCallback([&]() { std::lock_guard<std::mutex> _(lock); bar.SetTotal(r_total); bar.Update(r_count, r_rate); });
                return count >= total || should_stop;
            };
            bool success = false;
            std::thread job([&]() {
                success = scp.LoadFile(fout, remote);
                PostCallback([&]() { bar.Close(); });
            });
            bar.Run(true);
            job.join();
            if (!should_stop) {
	            PostCallback([=]() {
	                PromptOK(success ? Upp::DeQtf(t_("Transmition was failed!")) : Upp::DeQtf(t_("Transmition was completed!")));
	            });
            }
        }
    }
}

