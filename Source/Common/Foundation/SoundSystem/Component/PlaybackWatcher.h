#pragma once

#include "Foundation/Component/EventDispatcher.h"
#include "../HandleApi.h"

namespace MoonGlare::SoundSystem::Component {
using namespace MoonGlare::Component;

class PlaybackWatcher : public SoundSystem::iPlaybackWatcher {
    std::shared_ptr<Component::EventDispatcher> eventDispatcher;
public:
    PlaybackWatcher(std::shared_ptr<Component::EventDispatcher> eventDispatcher) : eventDispatcher(eventDispatcher) {}
    void OnFinished(SoundHandle handle, bool loop, UserData userData) override;
};

}
