#include <chrono>

#include "WorkThread.h"
#include "StateProcessor.h"

#include <libSpace/src/Container/StaticVector.h>

namespace MoonGlare::SoundSystem {

WorkThread::WorkThread(SoundSystem * owner, iFileSystem * fs) : soundSystem(owner), fileSystem(fs) { }

WorkThread::~WorkThread() { }

void WorkThread::Initialize() {
    if (thread.joinable())
        throw std::runtime_error("Already initialized");

    InitializeDevice();

    stateProcessor = std::make_unique<StateProcessor>(fileSystem);
    stateProcessor->Initialize();

    thread = std::thread([this]() { 
        threadState = ThreadState::Starting;
        ThreadMain(); 
        threadState = ThreadState::Finished;
    });
}

void WorkThread::Finalize() {
    if (threadState >= ThreadState::Running) {
        threadState = ThreadState::Stopping;
        thread.join();
        threadState = ThreadState::Stopped;
    }

    stateProcessor->Finalize();
    stateProcessor.reset();
    
    FinalizeDevice();
}

void WorkThread::ThreadMain() {
    OrbitLogger::ThreadInfo::SetName("SSWT");

    std::mutex mtx;
    threadState = ThreadState::Running;
    auto reportTime = std::chrono::steady_clock::now();
    while (threadState == ThreadState::Running) {
        stateProcessor->Step();

        std::unique_lock<std::mutex> lock(mtx);
        threadWait.wait_for(lock, std::chrono::milliseconds(Configuration::ThreadStep));

        auto currentTime = std::chrono::steady_clock::now();
        auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - reportTime).count();
        if (interval > Configuration::DebugReportInterval) {
            reportTime = currentTime;
            stateProcessor->PrintState();
        }
    }
}

void WorkThread::InitializeDevice() {
    /* Open and initialize a device */
    ALCdevice *device = alcOpenDevice(nullptr);
    ALCcontext *ctx = alcCreateContext(device, nullptr);
    if (ctx == nullptr || alcMakeContextCurrent(ctx) == ALC_FALSE)
    {
        if (ctx != nullptr)
            alcDestroyContext(ctx);
        alcCloseDevice(device);

        throw std::runtime_error("Cannot open sound device");
    }

    const ALCchar *name = nullptr;
    if (alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
        name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
    if (!name || alcGetError(device) != AL_NO_ERROR)
        name = alcGetString(device, ALC_DEVICE_SPECIFIER);

    AddLogf(Debug, "OpenAl initialized");
    AddLogf(System, "OpenAl version: %d.%d", alGetInteger(ALC_MAJOR_VERSION), alGetInteger(ALC_MINOR_VERSION));
    AddLogf(System, "Sound device used: %s", name);
    AddLogf(System, "Sound device extensions: %s", alcGetString(device, ALC_EXTENSIONS));
}

void WorkThread::FinalizeDevice() {
    ALCcontext *ctx = alcGetCurrentContext();
    if (ctx == nullptr)
        return;

    ALCdevice *device = alcGetContextsDevice(ctx);

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(ctx);
    alcCloseDevice(device);
}

}
