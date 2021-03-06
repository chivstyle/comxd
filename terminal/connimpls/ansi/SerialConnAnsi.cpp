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
	mCharsets[2] = mCharsets[0];
	mCharsets[3] = mCharsets[0];
	//
	AddAnsiControlSeqs(this->mSeqsFactory);
	//
	InstallFunctions();
}

void SerialConnAnsi::InstallFunctions()
{
	mFunctions[ANSI_RCP] = [=](const std::string_view& p) { this->LoadCursorData(this->mCursorData); };
	mFunctions[ANSI_SCP] = [=](const std::string_view& p) { this->SaveCursorData(this->mCursorData); };
}

void SerialConnAnsi::ProcessSS2(const std::string_view&)
{
	mCharset = mCharsets[2];
}
void SerialConnAnsi::ProcessSS3(const std::string_view&)
{
	mCharset = mCharsets[3];
}
uint32_t SerialConnAnsi::RemapCharacter(uint32_t uc, int charset)
{
	if (uc >= 0x80 && uc < 0xff) {
	    return Ansi_RemapCharacter(uc, charset);
	} else return SerialConnVT100::RemapCharacter(uc, charset);
}
