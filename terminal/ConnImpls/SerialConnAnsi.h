//
// (c) 2020 chiv
//
#pragma once

#include "SerialConnECMA48.h"

class SerialConnAnsi : public virtual SerialConnECMA48 {
public:
	SerialConnAnsi(std::shared_ptr<SerialIo> serial);
protected:
	virtual int IsControlSeq(const std::string& seq);
	virtual bool ProcessControlSeq(const std::string& seq, int seq_type);
	//
private:
	std::map<std::string, std::function<void()> > mAnsiTrivialHandlers;
	void InstallAnsiFunctions();
	//
	void ProcessAnsiTrivial(const std::string& seq);
	//
	Upp::Point mCursorXy;
};

