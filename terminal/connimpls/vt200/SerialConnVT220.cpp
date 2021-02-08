/*!
// (c) 2021 chiv
//
*/
#include "SerialConnVT220.h"
#include "VT220ControlSeq.h"
#include "VT220Charset.h"
#include "ConnFactory.h"

REGISTER_CONN_INSTANCE("vt220 by chiv", "vt220", SerialConnVT220);

using namespace Upp;

SerialConnVT220::SerialConnVT220(std::shared_ptr<SerialIo> io)
	: SerialConnVT100(io)
	, SerialConnVT(io)
	, mSelectiveErase(false)
{
	// vt100 supports G0,G1, vt200 supports G2,G3
	mCharsets[2] = CS_DEFAULT;
	mCharsets[3] = CS_DEFAULT;
	//
	SaveCursorData(mCursorData);
	//
	AddVT220ControlSeqs(this->mSeqsFactory);
	//
	InstallFunctions();
}

void SerialConnVT220::InstallFunctions()
{
	mFunctions[DECSCL] = [=](const std::string& p) { ProcessDECSCL(p); };
	mFunctions[G2_CS] = [=](const std::string& p) { ProcessG2_CS(p); };
	mFunctions[G3_CS] = [=](const std::string& p) { ProcessG3_CS(p); };
	mFunctions[S7C1T] = [=](const std::string& p) { ProcessS7C1T(p); };
	mFunctions[S8C1T] = [=](const std::string& p) { ProcessS8C1T(p); };
	mFunctions[DECSCA] = [=](const std::string& p) { ProcessDECSCA(p); };
	mFunctions[DECSEL] = [=](const std::string& p) { ProcessDECSEL(p); };
	mFunctions[DECSED] = [=](const std::string& p) { ProcessDECSED(p); };
	mFunctions[DECSTR] = [=](const std::string& p) { ProcessDECSTR(p); };
	mFunctions[SecondaryDA] = [=](const std::string& p) { ProcessSecondaryDA(p); };
}
// compatible level
void SerialConnVT220::ProcessDECSCL(const std::string& p)
{
	// TODO: Should we support 8-bit control seq ?
	if (p == "61") { // level1, vt100
	} else if (p == "62" || p == "62;2" || p == "62;0") { // level2, vt200, 8-bit controls
	} else if (p == "62;1") { // vt200, 7-bit controls
	}
}
#define DO_SET_CHARSET(g) do { \
	if (p == "<") { \
		mCharsets[g] = CS_DEC_SUPPLEMENTAL; \
	} else if (p == "A") { \
		mCharsets[g] = CS_BRITISH; \
	} else if (p == "4") { \
		mCharsets[g] = CS_DUTCH; \
	} else if (p == "5" || p == "C") { \
		mCharsets[g] = CS_FINNISH; \
	} else if (p == "R") { \
		mCharsets[g] = CS_FRENCH; \
	} else if (p == "Q") { \
		mCharsets[g] = CS_FRENCH_CANADIAN; \
	} else if (p == "K") { \
		mCharsets[g] = CS_GERMAN; \
	} else if (p == "Y") { \
		mCharsets[g] = CS_ITALIAN; \
	} else if (p == "E" || p == "6") { \
		mCharsets[g] = CS_DANISH; \
	} else if (p == "Z") { \
		mCharsets[g] = CS_SPANISH; \
	} else if (p == "H" || p == "7") { \
		mCharsets[g] = CS_SWEDISH; \
	} else if (p == "=") { \
		mCharsets[g] = CS_SWISS; \
	} else if (p == "B") { \
		mCharsets[g] = CS_ASCII; \
	} else if (p == "0") { \
		mCharsets[g] = CS_DEC_SPECIAL_GRAPHICS; \
	} else { \
		mCharsets[g] = CS_DEFAULT; \
	} \
	mCharset = mCharsets[g]; \
} while (0)
void SerialConnVT220::ProcessG0_CS(const std::string& p)
{
	DO_SET_CHARSET(0);
}
void SerialConnVT220::ProcessG1_CS(const std::string& p)
{
	DO_SET_CHARSET(1);
}
void SerialConnVT220::ProcessG2_CS(const std::string& p)
{
	DO_SET_CHARSET(2);
}
void SerialConnVT220::ProcessG3_CS(const std::string& p)
{
	DO_SET_CHARSET(3);
}
// S8C will break the UTF-8 sequences, so we do not support S8C.
void SerialConnVT220::ProcessS7C1T(const std::string&)
{
	SetUseS8C(false);
}
void SerialConnVT220::ProcessS8C1T(const std::string&)
{
	SetUseS8C(true);
}

void SerialConnVT220::ProcessDECSCA(const std::string& p)
{
	int ps = atoi(p.c_str());
	switch (ps) {
	case 0: this->SetDefaultStyle(); break;
	case 1: mSelectiveErase = false; break;
	case 2: mSelectiveErase = true; break;
	default: break;
	}
}
void SerialConnVT220::ProcessDECSEL(const std::string&)
{
    VTLine& vline = mLines[mVy];
	for (int i = mVx; i < (int)vline.size(); ++i) {
		vline[i] = ' ';
	}
}
void SerialConnVT220::ProcessDECSED(const std::string&)
{
	if (mSelectiveErase) {
		// erase char, do not erase the style
		VTLine& vline = mLines[mVy];
		for (int i = mVx; i < (int)vline.size(); ++i) {
			vline[i] = ' ';
		}
		for (int y = mVy+1; y < (int)mLines.size(); ++y) {
			for (int x = 0; x < (int)mLines[y].size(); ++x) {
				mLines[y][x] = ' ';
			}
		}
	}
}
void SerialConnVT220::ProcessDECSTR(const std::string&)
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
void SerialConnVT220::ProcessSecondaryDA(const std::string& p)
{
	int ps = atoi(p.c_str());
	switch (ps) {
	case 0:
		Put("\x1b[>1;10;0c"); // VT220 version 1.0, no options
		break;
	}
}
//
void SerialConnVT220::SaveCursorData(CursorDataVT220& cd)
{
	SerialConnVT100::SaveCursorData(cd);
	cd.DECOM = SerialConnVT100::mModes.DECOM;
	cd.SelectiveErase = mSelectiveErase;
}
void SerialConnVT220::LoadCursorData(const CursorDataVT220& cd)
{
	SerialConnVT100::LoadCursorData(cd);
	mSelectiveErase = cd.SelectiveErase;
	SerialConnVT100::mModes.DECOM = cd.DECOM;
}

void SerialConnVT220::ProcessDECDSR(const std::string& p)
{
	int ps = atoi(p.c_str());
	switch (ps) {
	case 26:
		Put("\x1b?27;1n"); // set keyboard language to "North American"
		break;
	default:
		SerialConnVT100::ProcessDECDSR(p);
		break;
	}
}

void SerialConnVT220::ProcessDECSM(const std::string& p)
{
	int ps = atoi(p.c_str());
	switch (ps) {
	case 25: SetShowCursor(true); break;
	case 42: mModes.DECNRCM = 1; break;
	default:
		return SerialConnVT100::ProcessDECSM(p);
	}
}
void SerialConnVT220::ProcessDECRM(const std::string& p)
{
	int ps = atoi(p.c_str());
	switch (ps) {
	case 25: SetShowCursor(false); break;
	case 42: mModes.DECNRCM = 0; break;
	default:
		return SerialConnVT100::ProcessDECRM(p);
	}
}
void SerialConnVT220::ProcessSS2(const std::string&)
{
	mCharset = mCharsets[2];
}
void SerialConnVT220::ProcessSS3(const std::string&)
{
	mCharset = mCharsets[3];
}
void SerialConnVT220::ProcessLS1R(const std::string&)
{
	mExtendedCharset = mCharsets[1];
}
void SerialConnVT220::ProcessLS2(const std::string&)
{
	mCharset = mCharsets[2];
}
void SerialConnVT220::ProcessLS3(const std::string&)
{
	mCharset = mCharsets[3];
}
void SerialConnVT220::ProcessLS2R(const std::string&)
{
	mExtendedCharset = mCharsets[2];
}
void SerialConnVT220::ProcessLS3R(const std::string&)
{
	mExtendedCharset = mCharsets[3];
}
//
uint32_t SerialConnVT220::RemapCharacter(uint32_t uc, int charset)
{
	return VT220_RemapCharacter(uc, charset, mExtendedCharset);
}

bool SerialConnVT220::ProcessKeyDown(Upp::dword key, Upp::dword flags)
{
	bool processed = false;
	if (flags == 0) {
		processed = true;
		switch (key) {
		case K_F6: Put("\E[17~"); break;
		case K_F7: Put("\E[18~"); break;
		case K_F8: Put("\E[19~"); break;
		case K_F9: Put("\E[20~"); break;
		case K_F10: Put("\E[21~"); break;
		case K_F11: Put("\E[23~"); break;
		case K_F12: Put("\E[24~"); break;
		case K_DELETE: Put("\E[3~"); break; // vt220, Remove
		case K_INSERT: Put("\E[2~"); break; // vt220, Insert
		case K_PAGEUP: Put("\E[5~"); break; // vt220, prev screen
		case K_PAGEDOWN: Put("\E[6~"); break; // vt220, next screen
		default:
			processed = false;
			break;
		}
	}
	return processed ? true : SerialConnVT100::ProcessKeyDown(key, flags);
}
