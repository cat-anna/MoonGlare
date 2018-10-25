#include <pch.h>

#include "SkinComponent.h"
#include "SkinComponentLuaWrap.h"
#include "SkinSystem.h"

#include "BoneAnimatorComponent.h"
#include "BoneAnimatorComponentLuaWrap.h"
#include "BoneAnimatorSystem.h"
#include "AnimationFinishedEvent.h"

#include "RendererRegister.h"

namespace MoonGlare::Component {

void RendererRegister::Register(ECSRegister &r) {
    r.Component<SkinComponent, SkinComponentLuaWrap>();
    r.Component<BoneAnimatorComponent, BoneAnimatorComponentLuaWrap>();

    r.System<SkinSystem>();
    r.System<BoneAnimatorSystem>();

    r.Event<AnimationFinishedEvent>();
}

}
