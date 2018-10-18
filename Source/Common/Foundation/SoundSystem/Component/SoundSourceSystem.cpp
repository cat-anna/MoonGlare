#include <InterfaceMap.h>

#include <Foundation/Component/EventDispatcher.h>

#include "SoundSourceComponent.h"
#include "SoundSourceSystem.h"
#include "SoundStreamFinishedEvent.h"
#include "SoundSourceComponentLuaWrap.h"

#include "../iSoundSystem.h"

namespace MoonGlare::SoundSystem::Component {

SoundSourceSystem::SoundSourceSystem(iSubsystemManager *subsystemManager)
    : iSubsystem(), subsystemManager(subsystemManager) {
}

SoundSourceSystem::~SoundSourceSystem() {
}

//---------------------------------------------------------

void SoundSourceSystem::Step(const SubsystemUpdateData &data) { }

//---------------------------------------------------------

}
