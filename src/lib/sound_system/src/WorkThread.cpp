#include "WorkThread.hpp"
#include "StateProcessor.hpp"
#include "build_configuration.hpp"
#include <chrono>
#include <orbit_logger.h>

namespace MoonGlare::SoundSystem {

WorkThread::WorkThread(SoundSystem *owner, iReadOnlyFileSystem *fs) : soundSystem(owner) {
    if (thread.joinable()) {
        throw std::runtime_error("Already initialized");
    }

    stateProcessor = std::make_unique<StateProcessor>(fs);
    stateProcessor->Initialize();

    thread = std::thread([this]() {
        threadState = ThreadState::Starting;
        ThreadMain();
        threadState = ThreadState::Finished;
    });
}

WorkThread::~WorkThread() {
    if (threadState >= ThreadState::Running) {
        threadState = ThreadState::Stopping;
        thread.join();
        threadState = ThreadState::Stopped;
    }

    stateProcessor->Finalize();
    stateProcessor.reset();
}

void WorkThread::ThreadMain() {
    OrbitLogger::ThreadInfo::SetName("SSWT");

    std::mutex mtx;
    threadState = ThreadState::Running;
    auto reportTime = std::chrono::steady_clock::now();
    while (threadState == ThreadState::Running) {
        stateProcessor->Step();

        std::unique_lock<std::mutex> lock(mtx);
        threadWait.wait_for(lock, std::chrono::milliseconds(kThreadStep));

        if constexpr (kDebugBuild) {
            auto currentTime = std::chrono::steady_clock::now();
            auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - reportTime).count();
            if (interval > kDebugReportInterval) {
                reportTime = currentTime;
                stateProcessor->PrintState();
            }
        }
    }
}

} // namespace MoonGlare::SoundSystem
