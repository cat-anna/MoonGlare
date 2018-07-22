#pragma once

namespace MoonGlare::SoundSystem {

struct Configuration {
    enum {
        MaxBuffers = 256,
        BufferGenCount = 16,

        DesiredBufferSize = 512 * 1024, //bytes

        MaxSources = 64,
        SourceAcivationQueue = 16,
        SourceGenCount = 8,
        MaxBuffersPerSource = 4,

        ThreadStep = 10, //ms
        DebugReportInterval = 10 * 1000, //ms
    };
};

}
