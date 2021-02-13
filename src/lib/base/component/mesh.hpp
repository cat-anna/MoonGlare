#pragma once

// #include <Renderer/Resources/MaterialManager.h>
// #include <Renderer/Resources/Mesh/MeshResource.h>
// #include <Renderer/Resources/ResourceManager.h>

#include "component_common.hpp"

namespace MoonGlare::Component {

struct Mesh : public ComponentBase<Mesh> {
    static constexpr ComponentId kComponentId = 6;
    static constexpr char kComponentName[] = "mesh";
    static constexpr bool kEditable = true;
    static constexpr bool kSerializable = true;

    bool active = true;
    bool cast_shadow = true;
    bool receive_shadow = true;

    // Renderer::MeshResourceHandle meshHandle = {};
    // Renderer::MaterialResourceHandle materialHandle = {};
    // bool Load(ComponentReader &reader, Entity owner);
};

#ifdef WANTS_TYPE_INFO

auto GetTypeInfo(Mesh *) {
    return AttributeMapBuilder<Mesh>::Start(Mesh::kComponentName)
        ->AddField("active", &Mesh::active)
        ->AddField("cast_shadow", &Mesh::cast_shadow)
        ->AddField("receive_shadow", &Mesh::receive_shadow);
}

#endif

#ifdef WANTS_SERIALIZATION

void to_json(nlohmann::json &j, const Mesh &p) {
    j = {
        {"active", p.active},
        {"cast_shadow", p.cast_shadow},
        {"receive_shadow", p.receive_shadow},
    };
}
void from_json(const nlohmann::json &j, Mesh &p) {
    j.at("active").get_to(p.active);
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