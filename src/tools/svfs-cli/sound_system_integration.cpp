#include "sound_system_integration.hpp"
#include "luainterface.h"
#include "sound_system/iSoundSystem.hpp"

namespace MoonGlare::Tools::VfsCli {

using namespace SoundSystem;

struct SoundSystemIntegration::Impl {
    Impl(SharedLua lua, iReadOnlyFileSystem *fs) : lua(lua), fs(fs) {
        InitHandleApiIntegration(lua->get_sol());
        auto &sol = lua->get_sol();

        auto classes = sol["classes"].get_or_create<sol::table>();

        sol["SoundSystem"] = this;
    }

    static void InitHandleApiIntegration(sol::state_view &sol) {
        auto cl = sol["classes"].get_or_create<sol::table>();

        cl.new_usertype<Impl>("SoundSystemIntegration",               //
                              "start_system", &Impl::StartSystem,     //
                              "stop_system", &Impl::Stopsystem,       //
                              "close", &Impl::Close,                  //
                              "open", &Impl::Open,                    //
                              "pause", &Impl::Pause,                  //
                              "stop", &Impl::Stop,                    //
                              "play", &Impl::Play,                    //
                              "get_duration", &Impl::GetDuration,     //
                              "get_position", &Impl::GetTimePosition, //
                              "set_loop", &Impl::SetLoop,             //
                              "get_loop", &Impl::GetLoop,             //

                              "active_handles", &Impl::GetActiveHandles //
        );
    }

    SharedLua lua;
    iReadOnlyFileSystem *fs;
    std::shared_ptr<iSoundSystem> ss_instance;
    std::unique_ptr<iHandleApi> ss_handle;

    std::vector<SoundHandle> handles;

    std::vector<SoundHandle> GetActiveHandles() {
        auto newend =
            std::remove_if(handles.begin(), handles.end(), [this](auto item) { return !IsSoundHandleValid(item); });
        handles.erase(newend, handles.end());
        return handles;
    }

    void StartSystem() {
        if (ss_instance) {
            return;
        }
        ss_instance = iSoundSystem::Create(fs);
        ss_handle = ss_instance->GetHandleApi();
    }

    void Stopsystem() {
        ss_handle.reset();
        ss_instance.reset();
    }

    void Play(SoundHandle handle) {
        StartSystem();
        ss_handle->Play(handle);
    }
    void Pause(SoundHandle handle) {
        StartSystem();
        ss_handle->Pause(handle);
    }
    void Stop(SoundHandle handle) {
        StartSystem();
        ss_handle->Stop(handle);
    }
    void SetLoop(SoundHandle handle, bool value) {
        StartSystem();
        ss_handle->SetLoop(handle, value);
    }
    bool GetLoop(SoundHandle handle) {
        StartSystem();
        return ss_handle->GetLoop(handle);
    }
    void Close(SoundHandle handle, bool ContinuePlaying = false) {
        StartSystem();
        ss_handle->Close(handle, ContinuePlaying);
    }
    SoundHandle Open(const char *uri, bool StartPlayback = true, bool ReleaseOnStop = true) {
        StartSystem();
        // auto h = ss_handle->Open(uri, StartPlayback, SoundKind::Auto, ReleaseOnStop);
        // handles.emplace_back(h);
        // return h;
        return {};
    }
    bool IsSoundHandleValid(SoundHandle handle) {
        StartSystem();
        return ss_handle->IsSoundHandleValid(handle);
    }
    float GetTimePosition(SoundHandle handle) {
        StartSystem();
        return ss_handle->GetTimePosition(handle);
    }
    float GetDuration(SoundHandle handle) {
        StartSystem();
        return ss_handle->GetDuration(handle);
    }
    void SetReleaseOnStop(SoundHandle handle, bool value) {
        StartSystem();
        ss_handle->SetReleaseOnStop(handle, value);
    }
};

SoundSystemIntegration::SoundSystemIntegration(SharedLua lua, iReadOnlyFileSystem *fs)
    : impl(std::make_unique<Impl>(lua, fs)) {
    //
}

SoundSystemIntegration::~SoundSystemIntegration() {
}

} // namespace MoonGlare::Tools::VfsCli