#include "pch.h"

#define NEED_MESH_BUILDER
#define NEED_MATERIAL_BUILDER

#include "../../nfRenderer.h"
#include "../../iAsyncLoader.h"
#include "../MeshResource.h"
#include "MeshUpdate.h"

#include "../MaterialManager.h"

namespace MoonGlare::Renderer::Resources::Loader {

void CustomMeshLoader::Do(ResourceLoadStorage &storage) {
    using ichannels = Renderer::Configuration::VAO::InputChannels;

    auto builder = owner.GetBuilder(storage.m_Queue, handle);

    auto &md = builder.meshData;

    builder.subMeshArray = meshArray;
    builder.subMeshMaterialArray = materialArray;

    builder.AllocateVAO();
    builder.vaoBuilder.BeginDataChange();

    builder.vaoBuilder.CreateChannel(ichannels::Vertex);
    builder.vaoBuilder.SetChannelData<float, 3>(ichannels::Vertex, (const float*)(&md.verticles[0]), md.verticles.size());

    builder.vaoBuilder.CreateChannel(ichannels::Texture0);
    builder.vaoBuilder.SetChannelData<float, 2>(ichannels::Texture0, (const float*)(&md.UV0[0]), md.UV0.size());

    builder.vaoBuilder.CreateChannel(ichannels::Normals);
    builder.vaoBuilder.SetChannelData<float, 3>(ichannels::Normals, (const float*)(&md.normals[0]), md.normals.size());

    builder.vaoBuilder.CreateChannel(ichannels::Index);
    builder.vaoBuilder.SetIndex(ichannels::Index, (const unsigned*)(&md.index[0]), md.index.size());

    builder.vaoBuilder.EndDataChange();
    builder.vaoBuilder.UnBindVAO();
}

} //namespace MoonGlare::Renderer::Resources::Loader 
