#pragma once


#include <OrbitLogger/src/OrbitLogger.h>

namespace OrbitLogger {
namespace LogChannels {
enum GlobalChanels : LogChannel {

	StarVFS = FirstUserChannel,

	StdErr = FirstUserChannel + 4,
	StdOut,


	Script = FirstUserChannel + 8,
	Insider,
	Performance,
	Resources,

};
}
}
