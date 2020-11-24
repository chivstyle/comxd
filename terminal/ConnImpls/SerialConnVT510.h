//
// (c) 2020 chiv
//
#pragma once

#include "SerialConnVT102.h"

class SerialConnVT510 : public virtual SerialConnVT102 {
public:
	SerialConnVT510(std::shared_ptr<SerialIo> serial);
protected:
	virtual int IsControlSeq(const std::string& seq);
	virtual bool ProcessControlSeq(const std::string& seq, int seq_type);
	//
private:
	std::map<std::string, std::function<void()> > mVT510TrivialHandlers;
	void InstallVT510Functions();
	//
	void ProcessVT510Trivial(const std::string& seq);
};

