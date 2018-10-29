#include <pch.h>

#include <Foundation/Component/iSubsystem.h>

#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/Resources/MaterialManager.h>
#include <Renderer/Resources/Mesh/MeshResource.h>
#include <Renderer/Renderer.h>

#include "SkinComponent.h"

namespace MoonGlare::Component {

bool SkinComponent::Load(ComponentReader &reader, Entity owner) {
    auto node = reader.node;
    std::string meshUri = node.child("Mesh").text().as_string("");
    std::string materialUri = node.child("Material").text().as_string("");
    if (meshUri.empty()) {
        AddLogf(Error, "Attempt to load nameless skin!");
        return false;
    }

    castShadow = node.child("CastShadow").text().as_bool(true);

    auto &rm = *reader.manager->GetInterfaceMap().GetInterface<Renderer::RendererFacade>()->GetResourceManager();

    meshHandle = rm.GetMeshManager().LoadMesh(meshUri);
    materialHandle = rm.GetMaterialManager().LoadMaterial(materialUri);                  

    if (reader.localRelationsCount > 0) {
        //TODO: limit is 64
        memcpy(bones, reader.localRelations, reader.localRelationsCount * sizeof(bones[0]));
        validBones = (uint8_t)reader.localRelationsCount;
    }

    return true;
}   

}
