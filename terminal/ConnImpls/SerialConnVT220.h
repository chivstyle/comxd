//
// (c) 2020 chiv
//
#pragma once

#include "SerialConnVT102.h"

class SerialConnVT220 : public virtual SerialConnVT102 {
public:
	SerialConnVT220(std::shared_ptr<SerialIo> serial);
protected:
	virtual int IsControlSeq(const std::string& seq);
	virtual bool ProcessControlSeq(const std::string& seq, int seq_type);
	//
private:
	// 2 modes more than VT102
	struct VT220Modes {
		enum TextCursorMode {
			Hide, Show
		};
		uint32_t TextCursor : 1; // Text cursor enable mode
		enum KeypadMode {
			DECKPAM, DECKPNM
		};
		uint32_t Keypad : 1; // keypad mode
		VT220Modes()
			: Keypad(DECKPAM)
			, TextCursor(Show)
		{
		}
	};
	VT220Modes mVT220Modes;
	//
	std::map<std::string, std::function<void()> > mVT220TrivialHandlers;
	void InstallVT220Functions();
	//
	void ProcessVT220Trivial(const std::string& seq);
};
