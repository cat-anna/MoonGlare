#pragma once
                       
#include <OrbitLogger/src/OrbitLogger.h>

namespace OrbitLogger {
namespace LogChannels {
enum GlobalChanels : LogChannel {

	StarVFS = FirstUserChannel,

	StdErr = FirstUserChannel + 4,
	StdOut,


	Insider = FirstUserChannel + 8,
	Script,
	ScriptCall,
	ScriptRuntime,
	Performance,
	Resources,
	FSEvent,
	Event,
};
}
}
