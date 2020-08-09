#pragma once

#include <Foundation/Component/ECSRegister.h>
#include <Foundation/Component/EventDispatcher.h>

#include <interface_map.h>

namespace MoonGlare::Component {

struct RendererRegister {
    static void Register(ECSRegister &r);
};

} // namespace MoonGlare::Component
