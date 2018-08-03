#include <libSpace/src/Utils/CompileMurmurhash3.h>

#include "SoundSystem.h"
#include "StateProcessor.h"

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

SoundSettings SoundSystem::GetSettings() {
    if (!workThread)
        return { };
    return workThread->GetStateProcessor()->GetSettings();
}

void SoundSystem::SetSettings(SoundSettings value) {
    if (!workThread)
        return;
    workThread->GetStateProcessor()->SetSettings(value);
}

Settings::ApplyMethod SoundSystem::ValueChanged(const std::string &key, Settings* siface) {
    if(!workThread)
        return Settings::ApplyMethod::Restart;

    auto hash = Space::Utils::MakeHash32(key.c_str());
    switch (hash) {
    case "Sound.Volume.Master"_Hash32:
    case "Sound.Volume.Music"_Hash32:
    case "Sound.Volume.Effect"_Hash32:
    case "Sound.Enabled"_Hash32: {
        SoundSettings ss = workThread->GetStateProcessor()->GetSettings();
        ss.masterVolume = siface->GetFloat("Sound.Volume.Master", ss.masterVolume);
        ss.musicVolume = siface->GetFloat("Sound.Volume.Music", ss.musicVolume);
        ss.effectVolume = siface->GetFloat("Sound.Volume.Effect", ss.effectVolume);
        ss.enabled = siface->GetBool("Sound.Enabled", ss.enabled);
        workThread->GetStateProcessor()->SetSettings(ss);
        return Settings::ApplyMethod::Immediate;
    }
    case "Sound.BitDepth"_Hash32:
        return Settings::ApplyMethod::Restart;
    }
    return Settings::ApplyMethod::DontCare;
}

void SoundSystem::Initialize(InterfaceMap &ifmap) {
    if (workThread)
        throw std::runtime_error("Already Initialized");

    iFileSystem *fs = ifmap.GetInterface<iFileSystem>();

    workThread = std::make_unique<WorkThread>(this, fs);
    workThread->Initialize();

    auto stt = ifmap.GetSharedInterface<Settings>();
    if (stt) {
        SoundSettings ss;
        ss.masterVolume = stt->GetFloat("Sound.Volume.Master", ss.masterVolume);
        ss.musicVolume = stt->GetFloat("Sound.Volume.Music", ss.musicVolume);
        ss.effectVolume = stt->GetFloat("Sound.Volume.Effect", ss.effectVolume);
        ss.enabled = stt->GetBool("Sound.Enabled", ss.enabled);
        ss.bitDepth = (BitDepth)stt->GetInt("Sound.BitDepth", (int)ss.bitDepth);
        stt->Subscribe(shared_from_this());

        for (auto &decoder : Decoder::iDecoderFactory::GetDecoders()) {
            decoder.decoderFactory->SetSetings(ss);
        }
        workThread->GetStateProcessor()->SetSettings(ss);
    }
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
