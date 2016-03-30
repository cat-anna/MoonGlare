#pragma once


#include <OrbitLogger/OrbitLogger.h>

namespace OrbitLogger {
namespace LogChannels {
enum GlobalChanels : LogChannel {

	StarVFS = FirstUserChannel,


	Script = FirstUserChannel + 8,
	Insider,
	Performance,
	Resources,

};
}
}
