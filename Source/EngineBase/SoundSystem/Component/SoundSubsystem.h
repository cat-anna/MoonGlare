#pragma once

#include "../HandleApi.h"

#include <EngineBase/Component/nfComponent.h>
#include <EngineBase/Component/iSubsystem.h>
#include <EngineBase/Component/ComponentCreatedEvent.h>

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

    void HandleEvent(const ComponentCreatedEvent &ev);
private:
    class PlaybackWatcher : public SoundSystem::iPlaybackWatcher {
        SoundSubsystem *sss;
    public:
        PlaybackWatcher(SoundSubsystem *sss) : sss(sss) {}
        void OnFinished(SoundHandle handle, bool loop, UserData userData) override {
            sss->OnPlaybackFinished(handle, loop, userData);
        }
    };

    HandleApi handleApi;
    ComponentArray *componentArray;
    iSubsystemManager *subsystemManager;
    PlaybackWatcher playbackWatcher{ this };

    void OnPlaybackFinished(SoundHandle handle, bool loop, UserData userData);

};

}
