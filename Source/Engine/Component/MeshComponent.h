#pragma once

#include <Renderer/Resources/MaterialManager.h>
#include <Renderer/Resources/Mesh/MeshResource.h>
#include <Renderer/Resources/ResourceManager.h>

namespace MoonGlare::Component {     

struct MeshComponent {
    //static constexpr uint32_t ComponentLimit = 1024*1024;
    static constexpr char* ComponentName = "Mesh";
    //using SortMethod = ...     

    Renderer::MeshResourceHandle meshHandle = {};
    Renderer::MaterialResourceHandle materialHandle = {};
    bool castShadow = true;

    bool Load(ComponentReader &reader, Entity owner);
};     

}
