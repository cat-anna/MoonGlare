#pragma once

#include <Foundation/iFileSystem.h>

#include "Decoder/iDecoder.h"
#include "Configuration.h"
#include "OpenAl.h"

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
    WorkThread(SoundSystem *owner, iFileSystem *fs);
    ~WorkThread();

    void Initialize();
    void Finalize();

    HandleApi GetHandleApi() { return HandleApi(stateProcessor.get()); }
private:
    std::thread thread;
    std::atomic<ThreadState> threadState = ThreadState::Stopped;
    std::condition_variable threadWait;
    SoundSystem *soundSystem = nullptr;
    iFileSystem *fileSystem = nullptr;

    std::unique_ptr<StateProcessor> stateProcessor;

    void ThreadMain();

    void InitializeDevice();
    void FinalizeDevice();
};

}
