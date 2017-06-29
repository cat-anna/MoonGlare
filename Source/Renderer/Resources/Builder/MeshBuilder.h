#pragma once

#include <Commands/CommandQueue.h>

#include "VAOBuilder.h"

namespace MoonGlare::Renderer::Resources::Builder {

class MeshBuilder final {
    using Conf = Configuration::Mesh;
    using ConfRes = Configuration::Resources;
public:
    //void SetDiffuseColor(const emath::fvec4 &col) {
        //m_MaterialPtr->m_DiffuseColor = col;
    //}
    //bool SetDiffuseMap(const std::string &URI);
    //bool SetDiffuseMap(const std::string &URI, Configuration::TextureLoad loadcfg);

    //template<typename T>
    //void SetDiffuseMapPixels(Commands::CommandQueue &q, const T* Pixels, const emath::usvec2 &size,
    //	Configuration::TextureLoad loadcfg, Device::PixelType pxtype, Commands::CommandKey key = Commands::CommandKey()) {
    //}
    //(expanded_data, emath::usvec2(width, height), tload, Renderer::Device::PixelType::LuminanceAlpha);

    //Material* m_MaterialPtr;

    void AllocateVAO() {
        if(*vaoBuilder.m_HandlePtr == Device::InvalidVAOHandle)
            queue.MakeCommandKey<Commands::VAOSingleAllocate>(currentKey, vaoBuilder.m_HandlePtr);
    }

    VAOBuilder vaoBuilder;

    Conf::SubMeshArray &subMeshArray;
    MeshResourceHandle handle;
    Commands::CommandQueue &queue;
    Commands::CommandKey currentKey;
};

//static_assert((sizeof(MaterialManager) % 16) == 0, "Invalid size!");
//static_assert(std::is_trivial<MaterialBuilder>::value, "must be trivial!");

} //namespace MoonGlare::Renderer::Resources::Builder
