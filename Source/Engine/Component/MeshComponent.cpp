#include <pch.h>

#include <Foundation/Component/iSubsystem.h>

#include "MeshComponent.h"

#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/Resources/MaterialManager.h>
#include <Renderer/Resources/Mesh/MeshResource.h>
#include <Renderer/Renderer.h>

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
