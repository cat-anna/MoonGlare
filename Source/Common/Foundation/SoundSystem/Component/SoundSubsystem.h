#pragma once

#include "../HandleApi.h"

#include <Foundation/Component/nfComponent.h>
#include <Foundation/Component/iSubsystem.h>
#include <Foundation/Component/ComponentEvents.h>

namespace MoonGlare::SoundSystem::Component {

using namespace MoonGlare::Component;

using MoonGlare::Component::Entity;

class SoundSubsystem : public iSubsystem {
public:
    explicit SoundSubsystem(iSubsystemManager *subsystemManager);
    virtual ~SoundSubsystem();

    void Step(const SubsystemUpdateData &data) override;

    bool Load(ComponentReader &reader, Entity parent, Entity owner) override;

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
