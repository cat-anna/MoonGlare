#include "SoundSystem.h"

namespace MoonGlare::SoundSystem {

iSoundSystem* iSoundSystem::CreateNew() {
    return new SoundSystem();
}

//------------------------------------------------------------------

SoundSystem::SoundSystem(){
}

SoundSystem::~SoundSystem() {
}

Settings SoundSystem::GetSettings() {
    return Settings();
}

void SoundSystem::SetSettings(Settings value) {
}

void SoundSystem::Initialize(iFileSystem * fs) {
    if (workThread)
        throw std::runtime_error("Already Initialized");

    workThread = std::make_unique<WorkThread>(this, fs);
    workThread->Initialize();
}

void SoundSystem::Finalize() {
    if (!workThread)
        throw std::runtime_error("Not Initialized");

    workThread->Finalize();
    workThread.reset();
}

std::unique_ptr<iSound> SoundSystem::OpenSound(const std::string &uri, bool start, SoundKind kind) {
    return workThread->OpenSound(uri, start, kind);
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
