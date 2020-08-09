#pragma once 

#include "../HandleApi.h"
#include <Foundation/Component/iSubsystem.h>

#include "SoundSourceComponent.h"
#include "SoundSourceComponentLuaWrap.h"

#include <SoundSourceComponent.x2c.h>

namespace MoonGlare::SoundSystem::Component {

HandleApi SoundSourceComponent::handleApi;
HandleApi SoundSourceComponentLuaWrap::handleApi;

//---------------------------------------------------------

bool SoundSourceComponent::Load(ComponentReader &reader, Entity owner) {
    x2c::SoundSystem::SoundSourceComponentData_t entry;
    entry.ResetToDefault();
    if (!reader.Read(entry))
        return false;

    auto ha = handleApi;

    soundHandle = ha.Open(entry.uri, false, entry.kind, false);
    ha.SetUserData(soundHandle, owner.GetIntValue());

    return true;
}

}
