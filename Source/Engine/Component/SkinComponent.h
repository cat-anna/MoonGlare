#pragma once

#include <Foundation/Component/Entity.h>
#include <Foundation/Component/iSubsystem.h>
#include <Foundation/Resources/Mesh.h>
#include <Foundation/Resources/SkeletalAnimation.h>

#include <Renderer/Resources/Mesh/VAOResource.h>
#include <Renderer/Resources/MaterialManager.h>

namespace MoonGlare::Component {

struct SkinComponent {
    static constexpr uint32_t ComponentLimit = 1024;
    static constexpr char* ComponentName = "Skin";

    Renderer::MeshResourceHandle meshHandle = {};
    Renderer::MaterialResourceHandle materialHandle = {};
    Renderer::VAOResourceHandle vaoHandle = {};

    const Resources::MeshData *meshData = nullptr;
    bool castShadow = true;

    uint8_t validBones = 0;
    Entity bones[Resources::Configuration::BoneCountLimit];

    bool Load(ComponentReader &reader, Entity owner);
};

}
