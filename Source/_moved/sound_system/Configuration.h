#pragma once

namespace MoonGlare::SoundSystem::Configuration {
    enum {
        MaxBuffers = 256,
        BufferGenCount = 16,

        DesiredBufferSize = 1024 * 1024, //bytes

        MaxSources = 64,
        SourceAcivationQueue = 16,
        SourceGenCount = 8,
        MaxBuffersPerSource = 4,

        ThreadStep = 10, //ms
        DebugReportInterval = 10 * 1000, //ms

    };
    static constexpr float EffectThresholdDuration = 5.0f;
}
