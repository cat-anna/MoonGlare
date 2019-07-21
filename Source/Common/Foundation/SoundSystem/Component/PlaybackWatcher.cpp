#include "PlaybackWatcher.h"

#include "SoundStreamFinishedEvent.h"

namespace MoonGlare::SoundSystem::Component {
using namespace MoonGlare::Component;

void PlaybackWatcher::OnFinished(SoundHandle handle, bool loop, UserData userData) {
    Entity e = Entity::FromIntValue(static_cast<Entity::IntValue_t>(userData));
    eventDispatcher->Queue(SoundStreamFinishedEvent{ e, e });
}

}
