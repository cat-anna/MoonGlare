#pragma once

#include "HandleApi.hpp"
#include "OpenAl.hpp"
#include "decoder/decoder.hpp"
#include "readonly_file_system.h"
#include "sound_system/configuration.hpp"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

namespace MoonGlare::SoundSystem {

class SoundSystem;
class StateProcessor;

enum class ThreadState {
    Stopped,
    Starting,
    Running,
    Finished,
    Stopping,
};

class WorkThread {
public:
    WorkThread(SoundSystem *owner, iReadOnlyFileSystem *fs);
    ~WorkThread();

    StateProcessor *GetStateProcessor() { return stateProcessor.get(); }

    std::unique_ptr<iHandleApi> GetHandleApi() { return std::make_unique<HandleApi>(stateProcessor.get()); }

private:
    std::thread thread;
    std::atomic<ThreadState> threadState = ThreadState::Stopped;
    std::condition_variable threadWait;
    SoundSystem *const soundSystem = nullptr;

    std::unique_ptr<StateProcessor> stateProcessor;

    void ThreadMain();
};

} // namespace MoonGlare::SoundSystem
