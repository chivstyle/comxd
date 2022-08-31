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
            int64 t_count = 0;
            String remote = ~es;
            // create ftp to upload
            FileIn fin(filename);
            SFtp ftp = mSession->CreateSFtp();
            // check
            SFtp::DirEntry info = ftp.GetInfo(remote);
            if (info.IsDirectory()) {
                remote += "/" + GetBasename(filename);
            }
            //
            ProgressDialog bar;
            bar.SetTotal(fin.GetSize());
            auto t1 = std::chrono::high_resolution_clock::now();
            ftp.WhenProgress = [&](int64 sz, int64 total) {
                double ts = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t1).count();
                t_rate = total / ts;
                t_count = sz;
                PostCallback([&]() { bar.Update(t_count, t_rate); });
                return true;
            };
            std::thread job([&]() {
                ftp.SaveFile(remote, fin);
                PostCallback([&]() { bar.Close(); });
            });
            bar.Run(true);
            job.join();
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
            // create ftp to upload
            SFtp ftp = mSession->CreateSFtp();
            // check
            SFtp::DirEntry info = ftp.GetInfo(remote);
            if (info.IsFile()) {
                filename += "/" + GetBasename(remote);
            } else {
                PromptOK(Upp::DeQtf(t_("Remote file does not exist or invalid")));
                return;
            }
            FileOut fout(filename);
            ProgressDialog bar;
            auto t1 = std::chrono::high_resolution_clock::now();
            ftp.WhenProgress = [&](int64 sz, int64 total) {
                double ts = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t1).count();
                r_rate = total / ts;
                r_total = total;
                r_count = sz;
                PostCallback([&]() { bar.SetTotal(r_total); bar.Update(r_count, r_rate); });
                return true;
            };
            std::thread job([&]() {
                ftp.LoadFile(fout, remote);
                PostCallback([&]() { bar.Close(); });
            });
            bar.Run(true);
            job.join();
        }
    }
}

