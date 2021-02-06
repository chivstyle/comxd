/*!
// (c) 2021 chiv
*/
#include "VT200ControlSeq.h"

#define kCSI "\x1b["
#define kDEC "\x1b[?"
#define kSS3 "\x1bO"

void AddVT200ControlSeqs(ControlSeqFactory* factory)
{
	REGISTER_SEQ(factory, DECSCL, kCSI, Ps, 0, "\x70");
	REGISTER_SEQ(factory, G2_CS,  "\x1b*", Gs, 0, "");
	REGISTER_SEQ(factory, G3_CS,  "\x1b+", Gs, 0, "");
	REGISTER_SEQ(factory, S7C1T,  "\x1b F", No, 0, "");
	REGISTER_SEQ(factory, S8C1T,  "\x1b G", No, 0, "");
	REGISTER_SEQ(factory, DECKPAM, "\x1b=", No, 0, "");
	REGISTER_SEQ(factory, DECKPNM, "\x1b>", No, 0, "");
	REGISTER_SEQ(factory, DECSCA,  kCSI, Ps, 0, "\"q");
	REGISTER_SEQ(factory, DECSEL,  kDEC, Ps, 0, "K");
	REGISTER_SEQ(factory, DECSED,  kDEC, Ps, 0, "J");
	// capture all printing sequences
	REGISTER_SEQ(factory, PRINTING,  kCSI, Gs, 0, "i");
	REGISTER_SEQ(factory, SecondaryDA,  "\x1b[>", Ps, 0, "c");
	REGISTER_SEQ(factory, DECSTR, kCSI, No, 0, "!p");
}
