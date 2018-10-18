#pragma once 

#include <Foundation/Component/ECSRegister.h>
#include <Foundation/Component/EventDispatcher.h>

#include <Foundation/InterfaceMap.h>

namespace MoonGlare::SoundSystem::Component {
using namespace MoonGlare::Component;

struct SoundSystemRegister {
    static void Register(ECSRegister &r);
    static void Install(InterfaceMap &ifaceMap);
};

}
