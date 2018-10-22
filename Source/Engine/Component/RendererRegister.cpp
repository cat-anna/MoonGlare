#include <pch.h>

#include "SkinComponent.h"
#include "SkinComponentLuaWrap.h"
#include "SkinSystem.h"

#include "RendererRegister.h"

namespace MoonGlare::Component {

void RendererRegister::Register(ECSRegister &r) {
    r.Component<SkinComponent, SkinComponentLuaWrap>();
    r.System<SkinSystem>();
    //r.Event<SoundStreamFinishedEvent>();
}

}
