#include "PlaybackWatcher.h"

#include "SoundStreamFinishedEvent.h"

namespace MoonGlare::SoundSystem::Component {
using namespace MoonGlare::Component;

void PlaybackWatcher::OnFinished(SoundHandle handle, bool loop, UserData userData) {
    Entity e = Entity::FromIntValue(userData);
    eventDispatcher->Queue(SoundStreamFinishedEvent{ e, e });
}

}
