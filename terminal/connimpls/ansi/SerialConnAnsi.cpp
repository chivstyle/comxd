/*!
// (c) 2021 chiv
//
*/
#include "SerialConnAnsi.h"
#include "AnsiControlSeq.h"
#include "AnsiCharset.h"
#include "ConnFactory.h"

REGISTER_CONN_INSTANCE("ansi by chiv", "ansi", SerialConnAnsi);

SerialConnAnsi::SerialConnAnsi(std::shared_ptr<SerialIo> io)
	: SerialConnVT100(io)
	, SerialConnVT(io)
{
	mCharsets[2] = CS_US;
	mCharsets[3] = CS_US;
	//
	AddAnsiControlSeqs(this->mSeqsFactory);
	//
	InstallFunctions();
}

void SerialConnAnsi::InstallFunctions()
{
	mFunctions[ANSI_RCP] = [=](const std::string& p) { this->LoadCursorData(this->mCursorData); };
	mFunctions[ANSI_SCP] = [=](const std::string& p) { this->SaveCursorData(this->mCursorData); };
}

void SerialConnAnsi::ProcessSS2(const std::string&)
{
	mCharset = mCharsets[2];
}
void SerialConnAnsi::ProcessSS3(const std::string&)
{
	mCharset = mCharsets[3];
}
uint32_t SerialConnAnsi::RemapCharacter(uint32_t uc, int charset)
{
	if (uc >= 0x80 && uc < 0xff) {
	    return Ansi_RemapCharacter(uc, charset);
	} else return SerialConnVT100::RemapCharacter(uc, charset);
}
