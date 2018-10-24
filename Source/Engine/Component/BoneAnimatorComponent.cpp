#include <pch.h>

#include <nfMoonGlare.h>
#include <Core/Component/SubsystemManager.h>

//#include <Renderer/Resources/ResourceManager.h>
//#include <Renderer/Resources/MaterialManager.h>
//#include <Renderer/Resources/Mesh/MeshResource.h>
//#include <Renderer/Renderer.h>
//#include <Renderer/Deferred/DeferredFrontend.h>

#include "BoneAnimatorComponent.h"

namespace MoonGlare::Component {

bool BoneAnimatorComponent::Load(ComponentReader &reader, Entity owner) {
    //auto node = reader.node;
    //std::string meshUri = node.child("Mesh").text().as_string("");
    //std::string materialUri = node.child("Material").text().as_string("");
    //if (meshUri.empty()) {
    //    AddLogf(Error, "Attempt to load nameless Mesh!");
    //    return false;
    //}

    //auto &rm = *dynamic_cast<Core::Component::SubsystemManager*>(reader.manager)->GetWorld()->GetRendererFacade()->GetResourceManager();

    //meshHandle = rm.GetMeshManager().LoadMesh(meshUri);
    //materialHandle = rm.GetMaterialManager().LoadMaterial(materialUri);

    return true;
}

}
