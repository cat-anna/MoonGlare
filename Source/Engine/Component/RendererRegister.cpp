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

#include "LightComponent.h"
#include "LightComponentLuaWrap.h"
#include "LightSystem.h"

#include "CameraComponent.h"
#include "CameraComponentLuaWrap.h"
#include "CameraSystem.h"

#include "RendererRegister.h"

namespace MoonGlare::Component {

void RendererRegister::Register(ECSRegister &r) {
    r.Component<SkinComponent, SkinComponentLuaWrap>();
    r.Component<BoneAnimatorComponent, BoneAnimatorComponentLuaWrap>();
    r.Component<MeshComponent, MeshComponentLuaWrap>();
    r.Component<LightComponent, LightComponentLuaWrap>();
    r.Component<CameraComponent, CameraComponentLuaWrap>();

    r.System<SkinSystem>();
    r.System<BoneAnimatorSystem>();
    r.System<MeshSystem>();
    r.System<LightSystem>();
    r.System<CameraSystem>();

    r.Event<AnimationFinishedEvent>();
}

}
