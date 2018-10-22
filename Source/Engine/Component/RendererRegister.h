#pragma once 

#include <Foundation/Component/ECSRegister.h>
#include <Foundation/Component/EventDispatcher.h>

#include <Foundation/InterfaceMap.h>

namespace MoonGlare::Component {

struct RendererRegister {
    static void Register(ECSRegister &r);
};

}
