#pragma once

#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/Resources/MaterialManager.h>
#include <Renderer/Resources/Mesh/MeshResource.h>

namespace MoonGlare::Component {     

struct MeshComponent {
    static constexpr char* ComponentName = "Mesh";
    //using SortMethod = ...     

    Renderer::MeshResourceHandle meshHandle = {};
    Renderer::MaterialResourceHandle materialHandle = {};
    bool castShadow = true;
   
    bool Load(ComponentReader &reader, Entity owner);
};     

}
