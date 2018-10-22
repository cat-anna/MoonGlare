#pragma once

#include <Foundation/Component/Entity.h>
#include <Foundation/Component/iSubsystem.h>

#include <Renderer/Mesh.h>
#include <Renderer/Resources/Mesh/MeshResource.h>
#include <Renderer/Resources/Mesh/VAOResource.h>
#include <Renderer/Resources/MaterialManager.h>

namespace MoonGlare::Component {

struct SkinComponent {
    static constexpr uint32_t ComponentLimit = 64;
    static constexpr char* ComponentName = "Skin";

    Renderer::MeshResourceHandle meshHandle = {};
    Renderer::MaterialResourceHandle materialHandle = {};
    Renderer::VAOResourceHandle vaoHandle = {};

    const Renderer::Resources::MeshData *meshData = nullptr;
    uint8_t validBones = 0;
    Entity bones[64];

    bool Load(ComponentReader &reader, Entity owner);
};

}
