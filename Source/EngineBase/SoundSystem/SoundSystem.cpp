#include "SoundSystem.h"

namespace MoonGlare::SoundSystem {

std::shared_ptr<iSoundSystem> iSoundSystem::Create() {
    static std::weak_ptr<iSoundSystem> weakPtr;
    auto ret = weakPtr.lock();
    if (!ret) {
        ret = std::make_shared<SoundSystem>();
        weakPtr = ret;
    }
    return ret;
}

//------------------------------------------------------------------

SoundSystem::SoundSystem() { }
SoundSystem::~SoundSystem() { 
    Finalize();
}

Settings SoundSystem::GetSettings() {
    assert(false);
    //todo
    return Settings();
}

void SoundSystem::SetSettings(Settings value) {
    assert(false);
    //todo
}

void SoundSystem::Initialize(iFileSystem * fs) {
    if (workThread)
        throw std::runtime_error("Already Initialized");

    workThread = std::make_unique<WorkThread>(this, fs);
    workThread->Initialize();
}

void SoundSystem::Finalize() {
    if (!workThread)
        return;

    workThread->Finalize();
    workThread.reset();
}

HandleApi SoundSystem::GetHandleApi() {
    return workThread->GetHandleApi();
}

std::vector<FormatInfo> SoundSystem::GetSupportedFormats() {
    std::vector<FormatInfo> r;
    for (auto &decoder : Decoder::iDecoderFactory::GetDecoders()) {
        for (auto format : decoder.supportedFormats) 
            r.emplace_back(format);
    }
    return r;
}

}
