/*!
// (c) 2022 chiv
//
// This program provides a versatile terminal, powered by TerminalCtrl [https://github.com/ismail-yilmaz/Terminal]
//
*/
#pragma once

#include "Conn.h"
#include "TerminalCtrl/Terminal/Terminal.h"
#include <thread>

class SerialConnVersatileTerm : public SerialConn {
public:
	SerialConnVersatileTerm(std::shared_ptr<SerialIo> io);
	virtual ~SerialConnVersatileTerm();
	
	// classic methods
	bool Start() override;
	void Stop() override;
	
	void GotFocus() override;
	
protected:
	void RxProc();
	
private:
	Upp::TerminalCtrl mVt;
	//
	volatile bool mRxShouldStop;
	std::thread mRxThr;
	//
	void InstallUserActions();
	void ShowVTOptions();
};
