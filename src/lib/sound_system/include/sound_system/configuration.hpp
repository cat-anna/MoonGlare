#pragma once

namespace MoonGlare::SoundSystem {

constexpr unsigned kMaxBuffers = 256;
constexpr unsigned kBufferGenCount = 16;
constexpr unsigned kDesiredBufferSize = 1024 * 1024; //bytes
constexpr unsigned kMaxSources = 64;
constexpr unsigned kSourceActivationQueue = 16;
constexpr unsigned kSourceGenCount = 8;
constexpr unsigned kMaxBuffersPerSource = 4;
constexpr unsigned kThreadStep = 10;                 //ms
constexpr unsigned kDebugReportInterval = 10 * 1000; //ms
constexpr float kEffectThresholdDuration = 5.0f;

} // namespace MoonGlare::SoundSystem
