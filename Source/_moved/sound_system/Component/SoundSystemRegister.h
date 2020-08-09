#pragma once

#include <Foundation/Component/ECSRegister.h>
#include <Foundation/Component/EventDispatcher.h>

#include <interface_map.h>

namespace MoonGlare::SoundSystem::Component {
using namespace MoonGlare::Component;

struct SoundSystemRegister {
    static void Register(ECSRegister &r);
    static void Install(InterfaceMap &ifaceMap);
};

} // namespace MoonGlare::SoundSystem::Component
