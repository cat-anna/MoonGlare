#include <pch.h>

#include "SkinComponent.h"
#include "SkinComponentLuaWrap.h"
#include "SkinSystem.h"

#include "BoneAnimatorComponent.h"
#include "BoneAnimatorComponentLuaWrap.h"
#include "BoneAnimatorSystem.h"
#include "AnimationFinishedEvent.h"

#include "MeshComponent.h"
#include "MeshComponentLuaWrap.h"
#include "MeshSystem.h"

#include "RendererRegister.h"

namespace MoonGlare::Component {

void RendererRegister::Register(ECSRegister &r) {
    r.Component<SkinComponent, SkinComponentLuaWrap>();
    r.Component<BoneAnimatorComponent, BoneAnimatorComponentLuaWrap>();
    r.Component<MeshComponent, MeshComponentLuaWrap>();

    r.System<SkinSystem>();
    r.System<BoneAnimatorSystem>();
    r.System<MeshSystem>();

    r.Event<AnimationFinishedEvent>();
}

}
