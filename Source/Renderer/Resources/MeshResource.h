#pragma once

#include "../nfRenderer.h"
#include "../Configuration.Renderer.h"
#include "../Configuration/Mesh.h"

#ifdef NEED_MESH_BUILDER
#include "Builder/MeshBuilder.h"
#endif

namespace MoonGlare::Renderer::Resources {

namespace Loader {
    class AssimpMeshLoader;
}

class 
    //alignas(16)
    MeshManager final 
    //: iAbstractResource
{
    friend class Loader::AssimpMeshLoader;

    using ThisClass = MeshManager;
    using Conf = Configuration::Mesh;
    using ConfRes = Configuration::Resources;

   // static constexpr VAOResourceHandle::Index_t GuardValue = 0xDEAD;
public:
    MeshManager(ResourceManager* Owner);
    ~MeshManager();

    using HandleType = MeshResourceHandle;

    bool Allocate(Commands::CommandQueue *q, HandleType &hout);
    void Release(Commands::CommandQueue *q, HandleType hin);

#ifdef NEED_MESH_BUILDER
    Builder::MeshBuilder GetBuilder(Commands::CommandQueue &q, HandleType &h, 
        Commands::CommandKey key = {}, bool AllowAllocation = false) {
        if (!IsHandleValid(h)) {
            if (!AllowAllocation)
                RendererAssert(false);//TODO:

            if(!Allocate(&q, h)) {
                RendererAssert(false);//TODO:
            }
        }


        return Builder::MeshBuilder{
            Builder::VAOBuilder {
                &q,
                &vaoBuffer[h.index][0],
                &deviceHandle[h.index],
            },
            subMesh[h.index],

            h,
       //     this,
            q,
            key,
        };
}
#endif

    bool LoadMesh(const std::string &uri, HandleType &hout);

    bool IsHandleValid(HandleType &h) const;

    auto GetMeshes(HandleType h) {
        if (!IsHandleValid(h)) {
            //TODO
            __debugbreak();
            throw false;
        }
        return subMesh[h.index];
    }
private:
    template<typename T>
    using Array = std::array<T, Conf::Limit>;

    using Bitmap = ConfRes::BitmapAllocator<Conf::Limit>;

    Bitmap allocationBitmap;
    Array<Device::VAOHandle> deviceHandle;
    Array<Conf::SubMeshArray> subMesh;
    Array<Conf::VAOBuffers> vaoBuffer;
    Array<HandleType::Generation_t> generations;

    //TODO: Array<material> material;
    //TODO: source hash?
    //TODO: storage for verticles, uvs, normals, etc

    std::unordered_map<std::string, HandleType> loadedMeshes; //temporary solution
    ResourceManager *resourceManager = nullptr;
};

//static_assert((sizeof(MaterialManager) % 16) == 0, "Invalid size!");

} //namespace MoonGlare::Renderer::Resources 
