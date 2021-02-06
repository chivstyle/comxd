/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT200.h"
#include "VT200ControlSeq.h"
#include "VT200Charset.h"
#include "ConnFactory.h"

REGISTER_CONN_INSTANCE("vt200 by chiv", "vt200", SerialConnVT200);

SerialConnVT200::SerialConnVT200(std::shared_ptr<SerialIo> io)
	: SerialConnVT100(io)
	, SerialConnVT(io)
	, mSelectiveErase(false)
	, mKeypadMode(KM_DECKPNM)
{
	// vt100 supports G0,G1, vt200 supports G2,G3
	mCharsets[2] = CS_US; // G2
	mCharsets[3] = CS_US; // G3
	//
	SaveCursorData(mCursorData);
	//
	AddVT200ControlSeqs(this->mSeqsFactory);
	//
	InstallFunctions();
}

void SerialConnVT200::InstallFunctions()
{
	mFunctions[DECSCL] = [=](const std::string& p) { ProcessDECSCL(p); };
	mFunctions[G2_CS] = [=](const std::string& p) { ProcessG2_CS(p); };
	mFunctions[G3_CS] = [=](const std::string& p) { ProcessG3_CS(p); };
	mFunctions[S7C1T] = [=](const std::string& p) { ProcessS7C1T(p); };
	mFunctions[S8C1T] = [=](const std::string& p) { ProcessS8C1T(p); };
	mFunctions[DECKPAM] = [=](const std::string& p) { ProcessDECKPAM(p); };
	mFunctions[DECKPNM] = [=](const std::string& p) { ProcessDECKPNM(p); };
	mFunctions[DECSCA] = [=](const std::string& p) { ProcessDECSCA(p); };
	mFunctions[DECSEL] = [=](const std::string& p) { ProcessDECSEL(p); };
	mFunctions[DECSED] = [=](const std::string& p) { ProcessDECSED(p); };
	mFunctions[DECSTR] = [=](const std::string& p) { ProcessDECSTR(p); };
	mFunctions[SecondaryDA] = [=](const std::string& p) { ProcessSecondaryDA(p); };
}

void SerialConnVT200::ProcessDECSCL(const std::string&)
{
}
void SerialConnVT200::ProcessG2_CS(const std::string&)
{
}
void SerialConnVT200::ProcessG3_CS(const std::string&)
{
}
void SerialConnVT200::ProcessS7C1T(const std::string&)
{
}
void SerialConnVT200::ProcessS8C1T(const std::string&)
{
}
void SerialConnVT200::ProcessDECKPAM(const std::string&)
{
}
void SerialConnVT200::ProcessDECKPNM(const std::string&)
{
}
void SerialConnVT200::ProcessDECSCA(const std::string&)
{
}
void SerialConnVT200::ProcessDECSEL(const std::string&)
{
}
void SerialConnVT200::ProcessDECSED(const std::string&)
{
}
void SerialConnVT200::ProcessDECSTR(const std::string&)
{
	// default state
	SetShowCursor(true);
	SerialConnVT100::mModes.DECAWM = VT100Modes::OFF;
	SetWrapLine(false);
	this->mScrollingRegion.Top = 0;
	this->mScrollingRegion.Bottom = 23;
	//
	Clear();
}
void SerialConnVT200::ProcessSecondaryDA(const std::string& p)
{
	int ps = atoi(p.c_str());
	switch (ps) {
	case 0:
		GetIo()->Write("\x1b[>1;10;0c"); // VT220 version 1.0, no options
		break;
	}
}
//
void SerialConnVT200::SaveCursorData(CursorDataVT200& cd)
{
	SerialConnVT100::SaveCursorData(cd);
	cd.DECOM = SerialConnVT100::mModes.DECOM;
	cd.SelectiveErase = mSelectiveErase;
}
void SerialConnVT200::LoadCursorData(const CursorDataVT200& cd)
{
	SerialConnVT100::LoadCursorData(cd);
	mSelectiveErase = cd.SelectiveErase;
	SerialConnVT100::mModes.DECOM = cd.DECOM;
}

void SerialConnVT200::ProcessDECDSR(const std::string& p)
{
	int ps = atoi(p.c_str());
	switch (ps) {
	case 26:
		GetIo()->Write(std::string("\x1b?27;1n")); // set keyboard language to "North American"
		break;
	default:
		SerialConnVT100::ProcessDECDSR(p);
		break;
	}
}

void SerialConnVT200::ProcessDECSM(const std::string& p)
{
	int ps = atoi(p.c_str());
	switch (ps) {
	case 25: SetShowCursor(true); break;
	case 42: mModes.DECNRCM = 1; break;
	default:
		return SerialConnVT100::ProcessDECSM(p);
	}
}
void SerialConnVT200::ProcessDECRM(const std::string& p)
{
	int ps = atoi(p.c_str());
	switch (ps) {
	case 25: SetShowCursor(false); break;
	case 42: mModes.DECNRCM = 0; break;
	default:
		return SerialConnVT100::ProcessDECSM(p);
	}
}
void SerialConnVT200::ProcessSS2(const std::string&)
{
	mCharset = mCharsets[2];
}
void SerialConnVT200::ProcessSS3(const std::string&)
{
	mCharset = mCharsets[3];
}

uint32_t SerialConnVT200::RemapCharacter(uint32_t uc, int charset)
{
	return SerialConnVT100::RemapCharacter(uc, charset);
}
