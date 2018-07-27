#pragma once

#include "../HandleApi.h"

#include <EngineBase/Component/nfComponent.h>
#include <EngineBase/Component/iSubsystem.h>

namespace MoonGlare::SoundSystem::Component {

using namespace MoonGlare::Component;

using MoonGlare::Component::Entity;
using MoonGlare::Component::Handle;

class SoundSubsystem : public iSubsystem {
public:
    explicit SoundSubsystem(iSubsystemManager *subsystemManager);
    virtual ~SoundSubsystem();

    void Update(const SubsystemUpdateData &data) override;

    bool Load(pugi::xml_node node, Entity Owner, Handle &hout) override;
private:
    class PlaybackWatcher : public SoundSystem::iPlaybackWatcher {
        SoundSubsystem *sss;
    public:
        PlaybackWatcher(SoundSubsystem *sss) : sss(sss) {}
        void OnFinished(SoundHandle handle, UserData userData) override {
            sss->OnPlaybackFinished(handle, userData);
        }
        void OnLoop(SoundHandle handle, UserData userData) override {
            sss->OnPlaybackLoop(handle, userData);
        }
    };

    HandleApi handleApi;
    ComponentArray *componentArray;
    PlaybackWatcher playbackWatcher{ this };

    void OnPlaybackFinished(SoundHandle handle, UserData userData);
    void OnPlaybackLoop(SoundHandle handle, UserData userData);
};

}
