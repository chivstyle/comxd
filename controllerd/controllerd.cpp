//
// (c) 2021 chiv
//
#include <Core/Core.h>
#include <signal.h>
#include "BusinessLogic.h"

using namespace Upp;

static volatile bool q_should_exit = false;

void on_signal(int sig)
{
	if (sig == SIGINT || sig == SIGABRT) {
		q_should_exit = true;
	}
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT);
	// install signal proc
	signal(SIGINT, on_signal);
	signal(SIGABRT, on_signal);
	//
	if (CommandLine().GetCount() < 1) {
		LOG("Usage: " << GetArgv0() << " <conf_file>");
	} else {
		try {
			BusinessLogic bl(CommandLine()[0]);
			bl.Run(&q_should_exit);
		} catch (const std::exception& ex) {
			LOG("exception:" << ex.what());
		}
	}
}
