#pragma once

#include <orbit_logger.h>

namespace OrbitLogger {
namespace LogChannels {
enum GlobalChanels : LogChannel {

    StarVFS = FirstUserChannel,

    StdErr = FirstUserChannel + 4,
    StdOut,

    Verbose = FirstUserChannel + 8,
    Script,
    ScriptCall,
    ScriptRuntime,
    Performance,
    Resources,
    FSEvent,
    Event,
    Sound,
    Test,
};
}
} // namespace OrbitLogger
