#include "../../nfRenderer.h"
#include "../../Configuration.Renderer.h"
#include "../../Configuration/Mesh.h"

#include "MeshBuilder.h"
#include "MeshResource.h"

#include <Commands/ResourceCommands.h>
#include <Commands/MemoryCommands.h>

namespace MoonGlare::Renderer::Resources::Builder {

void MeshBuilder::AllocateVAO() {
    if (*vaoBuilder.m_HandlePtr == Device::InvalidVAOHandle)
        queue.MakeCommandKey<Commands::VAOSingleAllocate>(currentKey, vaoBuilder.m_HandlePtr);
}

void MeshBuilder::Commit() {
    queue.MakeCommand<Commands::CommitMesh>(handle, meshManager);
}

void MeshBuilder::UpdateVAO() {
    using ichannels = Renderer::Configuration::VAO::InputChannels;

    auto &md = meshData;

    AllocateVAO();
    vaoBuilder.BeginDataChange();
    
    if (!md.verticles.empty()) {
        vaoBuilder.CreateChannel(ichannels::Vertex);
        vaoBuilder.SetChannelData<float, 3>(ichannels::Vertex, (const float*)(&md.verticles[0]), md.verticles.size());
    }
    
    if (!md.UV0.empty()) {
        vaoBuilder.CreateChannel(ichannels::Texture0);
        vaoBuilder.SetChannelData<float, 2>(ichannels::Texture0, (const float*)(&md.UV0[0]), md.UV0.size());
    }
    
    if (!md.normals.empty()) {
        vaoBuilder.CreateChannel(ichannels::Normals);
        vaoBuilder.SetChannelData<float, 3>(ichannels::Normals, (const float*)(&md.normals[0]), md.normals.size());
    }
    

    if (!md.index.empty()) {
        vaoBuilder.CreateChannel(ichannels::Index);
        vaoBuilder.SetIndex(ichannels::Index, (const unsigned*)(&md.index[0]), md.index.size());
    }
    
    vaoBuilder.EndDataChange();
    vaoBuilder.UnBindVAO();
}

void MeshBuilder::Set(const Mesh &meshes) {
    subMeshArray = {};
    queue.MakeCommand<Commands::MemoryStore<Mesh>>(meshes, &subMeshArray);
}

void MeshBuilder::Set(MaterialResourceHandle materials) {
    subMeshMaterialArray = {};
    queue.MakeCommand<Commands::MemoryStore<MaterialResourceHandle>>(materials, &subMeshMaterialArray);
}

} //namespace MoonGlare::Renderer::Resources::Builder
