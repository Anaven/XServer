#ifndef __RUN_SERVER__
#define __RUN_SERVER__

#include "Headers.h"
#include "XServer.h"
#include "ConfigPrefsParser.h"
  
//
// This function starts the Streaming Server. Pass in a source
// for preferences, a source for text messages, and an optional
// port to override the default.
//
// Returns the server state upon completion of startup. If this
// is qtssFatalErrorState, something went horribly wrong and caller
// should just die.
int StartServer(ConfigPrefsParser *inConfigPrefsParser, UInt16 inPort, UInt32 inInitialState, bool inDontFork);

//
// Call this after StartServer if it doesn't return qtssFatalError.
// This will not return until the server is going away
void RunServer();

// write pid to file
void WritePid(bool forked);

// clean the pid file
void CleanPid(bool force);

#endif

