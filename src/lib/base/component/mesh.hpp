#pragma once

// #include <Renderer/Resources/MaterialManager.h>
// #include <Renderer/Resources/Mesh/MeshResource.h>
// #include <Renderer/Resources/ResourceManager.h>

#include "component_common.hpp"

namespace MoonGlare::Component {

struct Mesh {
    static constexpr ComponentId kComponentId = 5;
    static constexpr char *kComponentName = "Mesh";
    static constexpr bool kEditable = true;
    static constexpr bool kSerializable = true;

    bool cast_shadow = true;
    bool receive_shadow = true;

    // Renderer::MeshResourceHandle meshHandle = {};
    // Renderer::MaterialResourceHandle materialHandle = {};
    // bool Load(ComponentReader &reader, Entity owner);
};

#ifdef _WANTS_TYPE_INFO_

auto GetTypeInfo(Mesh *) {
    return AttributeMapBuilder<Mesh>::Start("Mesh")
        ->AddField("cast_shadow", &Mesh::cast_shadow)
        ->AddField("receive_shadow", &Mesh::receive_shadow)
        // ->AddField()
        // ->AddField()
        // ->AddField()
        ;
}

#endif

#ifdef _WANTS_COMPONENT_SERIALIZATION_

void to_json(nlohmann::json &j, const Mesh &p) {
    j = {
        {"cast_shadow", p.cast_shadow},
        {"receive_shadow", p.receive_shadow},
    };
}
void from_json(const nlohmann::json &j, Mesh &p) {
    j.at("receive_shadow").get_to(p.receive_shadow);
    j.at("cast_shadow").get_to(p.cast_shadow);
}

#endif

} // namespace MoonGlare::Component

#if 0

#include <pch.h>

#include <Foundation/Component/iSubsystem.h>

#include "MeshComponent.h"

#include <Renderer/Renderer.h>
#include <Renderer/Resources/MaterialManager.h>
#include <Renderer/Resources/Mesh/MeshResource.h>
#include <Renderer/Resources/ResourceManager.h>

namespace MoonGlare::Component {
     
bool MeshComponent::Load(ComponentReader &reader, Entity owner) {
    auto node = reader.node;
    std::string meshUri = node.child("Mesh").text().as_string("");
    std::string materialUri = node.child("Material").text().as_string("");
    if (meshUri.empty()) {
        AddLogf(Error, "Attempt to load nameless Mesh!");
        return false;
    }
    castShadow = node.child("CastShadow").text().as_bool(true);

    auto &rm = *reader.manager->GetInterfaceMap().GetInterface<Renderer::RendererFacade>()->GetResourceManager();

    meshHandle = rm.GetMeshManager().LoadMesh(meshUri);
    materialHandle = rm.GetMaterialManager().LoadMaterial(materialUri);

    return true;
}

}
#endif