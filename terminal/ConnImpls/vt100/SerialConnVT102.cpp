/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT102.h"
#include "VT102ControlSeq.h"
#include "ConnFactory.h"

REGISTER_CONN_INSTANCE("VT102,chiv", "vt102", SerialConnVT102);

using namespace Upp;

SerialConnVT102::SerialConnVT102(std::shared_ptr<SerialIo> io)
	: SerialConnVT(io)
	, SerialConnEcma48(io)
{
	AddVT102ControlSeqs(this->mSeqsFactory);
	// VT102, permanently selected modes
	SerialConnEcma48::mModes.CRM = 0;
	SerialConnEcma48::mModes.EBM = 0;
	SerialConnEcma48::mModes.ERM = 1;
	SerialConnEcma48::mModes.FEAM = 0;
	SerialConnEcma48::mModes.PUM = 0;
	SerialConnEcma48::mModes.SRTM = 0;
	SerialConnEcma48::mModes.TSM = 0;
	//
	SaveCursor(mCursorData);
	//
	InstallFunctions();
}

void SerialConnVT102::InstallFunctions()
{
	mFunctions[VT102_MODE_SET] = [=](const std::string& p) { ProcessVT102_MODE_SET(p); };
	mFunctions[VT102_MODE_RESET] = [=](const std::string& p) { ProcessVT102_MODE_RESET(p); };
	mFunctions[VT102_DSR] = [=](const std::string& p) { ProcessVT102_DSR(p); };
	//
	mFunctions[DECSTBM] = [=](const std::string& p) { ProcessDECSTBM(p); };
	mFunctions[DECSC] = [=](const std::string& p) { ProcessDECSC(p); };
	mFunctions[DECRC] = [=](const std::string& p) { ProcessDECRC(p); };
	mFunctions[DECALN] = [=](const std::string& p) { ProcessDECALN(p); };
	mFunctions[DECTST] = [=](const std::string& p) { ProcessDECTST(p); };
	mFunctions[DECLL] = [=](const std::string& p) { ProcessDECLL(p); };
}

void SerialConnVT102::ProcessDA(const std::string& p)
{
    int ps = atoi(p.c_str());
    switch (ps) {
    case 0:
        GetIo()->Write("\x1b[?6c"); // I'm VT102
        break;
    }
}
void SerialConnVT102::ProcessDSR(const std::string& p)
{
	int ps = atoi(p.c_str());
	switch (ps) {
	case 5:
		GetIo()->Write("\x1b[0n"); // It's OK
		break;
	case 6: if (1) {
		std::string cpr = std::string("\x1b[") + std::to_string(mPy/mFontH)
		    + ";" + std::to_string(mPx/mFontW);
		GetIo()->Write(cpr);
	} break;
	}
}

void SerialConnVT102::ProcessVT102_MODE_SET(const std::string& p)
{
    int ps = atoi(p.c_str());
    switch (ps) {
    case 1:  mModes.DECCKM  = 1; break;
    case 3:  mModes.DECCOLM = 1; break;
    case 4:  mModes.DECSCLM = 1; break;
    case 5:  mModes.DECSCNM = 1; break;
    case 6:  mModes.DECOM   = 1; break;
    case 7:  mModes.DECAWM  = 1; break;
    case 8:  mModes.DECARM  = 1; break;
    case 18: mModes.DECPFF  = 1; break;
    case 19: mModes.DECPEX  = 1; break;
    }
}
void SerialConnVT102::ProcessVT102_MODE_RESET(const std::string& p)
{
    int ps = atoi(p.c_str());
    switch (ps) {
    case 1:  mModes.DECCKM  = 0; break;
    case 2:  mModes.DECANM  = 0; break;
    case 3:  mModes.DECCOLM = 0; break;
    case 4:  mModes.DECSCLM = 0; break;
    case 5:  mModes.DECSCNM = 0; break;
    case 6:  mModes.DECOM   = 0; break;
    case 7:  mModes.DECAWM  = 0; break;
    case 8:  mModes.DECARM  = 0; break;
    case 18: mModes.DECPFF  = 0; break;
    case 19: mModes.DECPEX  = 0; break;
    }
}

void SerialConnVT102::ProcessVT102_DSR(const std::string& p)
{
	int ps = atoi(p.c_str());
	switch (ps) {
	case 15:
		GetIo()->Write("\x1b[?13n"); // No printer
		break;
	}
}

void SerialConnVT102::ProcessCUP(const std::string& p)
{
	if (p.empty()) {
		if (mModes.DECOM == VT102Modes::DECOM_Absolute) {
			mVx = 0;
			mVy = 0;
		} else {
			mVx = 0;
			mVy = mScrollingRegion.Top;
		}
	} else {
		SerialConnEcma48::ProcessCUP(p);
	}
}
//
void SerialConnVT102::ProcessHVP(const std::string& p)
{
	ProcessCUP(p);
}
//
void SerialConnVT102::ProcessVT102_IND(const std::string&)
{
	int bot = mScrollingRegion.Bottom;
	if (bot < 0) bot = (int)mLines.size()-1;
	if (mVy == bot) { // scroll up
		int top = mScrollingRegion.Top;
		auto it_end = mLines.begin() + bot + 1;
		Size csz = GetConsoleSize();
		mLines.insert(it_end, VTLine(csz.cx, mBlankChar).SetHeight(mFontH));
		mLines.erase(mLines.begin() + mVx);
	} else {
		mVy++;
	}
}

void SerialConnVT102::ProcessDECSTBM(const std::string& p)
{
    Size csz = GetConsoleSize();
    int idx = 0, pn[2] = {1, 1};
    SplitString(p.c_str(), ';', [=, &idx, &pn](const char* token) {
        if (idx < 2)
            pn[idx++] = atoi(token);
    });
    if (pn[0] <= 0) pn[0] = 1; //
    if (pn[1] <= 0) pn[1] = 1; //
    if (pn[0] < pn[1] && pn[1] <= csz.cy) {
        mScrollingRegion.Top = pn[0]-1;
        mScrollingRegion.Bottom = pn[1]-1;
    }
}
void SerialConnVT102::SaveCursor(CursorData& cd)
{
	cd.Vx = mVx;
	cd.Vy = mVy;
	cd.Px = mPx;
	cd.Py = mPy;
	cd.Style = mStyle;
}
void SerialConnVT102::LoadCursor(const CursorData& cd)
{
	mVx = cd.Vx;
	mVy = cd.Vy;
	mPx = cd.Px;
	mPy = cd.Py;
	mStyle = cd.Style;
}
void SerialConnVT102::ProcessDECSC(const std::string&)
{
    SaveCursor(mCursorData);
}
void SerialConnVT102::ProcessDECRC(const std::string& p)
{
	LoadCursor(mCursorData);
}

void SerialConnVT102::ProcessDECALN(const std::string&)
{
	for (size_t vy = 0; vy < mLines.size(); ++vy) {
		VTLine& vline = mLines[vy];
		for (size_t vx = 0; vx < vline.size(); ++vx) {
			vline[vx] = 'S';
		}
	}
}
void SerialConnVT102::ProcessDECTST(const std::string& p)
{
}
void SerialConnVT102::ProcessDECLL(const std::string& p)
{
}
