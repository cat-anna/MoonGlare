#pragma once

#include "../nfRenderer.h"
#include "../Configuration.Renderer.h"
#include "../Configuration/Mesh.h"

#ifdef NEED_MESH_BUILDER
#include "Builder/MeshBuilder.h"
#endif

namespace MoonGlare::Renderer::Resources {

struct MeshData {
    std::vector<glm::fvec3> verticles;
    std::vector<glm::fvec2> UV0;
    std::vector<glm::fvec3> normals;
    std::vector<uint32_t> index;
};

class 
    //alignas(16)
    MeshManager final 
    //: iAbstractResource
{
    using ThisClass = MeshManager;
    using Conf = Configuration::Mesh;
    using ConfRes = Configuration::Resources;

   // static constexpr VAOResourceHandle::Index_t GuardValue = 0xDEAD;
public:
    MeshManager(ResourceManager* Owner);
    ~MeshManager();

    ResourceManager* GetResourceManager() { return resourceManager; }

    using HandleType = MeshResourceHandle;

    bool Allocate(HandleType &hout);
    void Release(HandleType hin);
                                    
#ifdef NEED_MESH_BUILDER
    Builder::MeshBuilder GetBuilder(Commands::CommandQueue &q, HandleType &h, 
        Commands::CommandKey key = {}, bool AllowAllocation = false) {
        if (!IsHandleValid(h)) {
            if (!AllowAllocation)
                RendererAssert(false);//TODO:

            if(!Allocate(h)) {
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
            materialHandle[h.index],
            h,
            meshData[h.index],
            q,
            key,
        };
}
#endif

    bool LoadMesh(const std::string &uri, HandleType &hout);

    bool IsHandleValid(HandleType &h) const;

    auto& GetMeshes(HandleType h) {
        if (!IsHandleValid(h)) {
            //TODO
            __debugbreak();
            throw false;
        }
        return subMesh[h.index];
    }
    auto& GetMaterials(HandleType h) {
        if (!IsHandleValid(h)) {
            //TODO
            __debugbreak();
            throw false;
        }
        return materialHandle[h.index];
    }

private:
    template<typename T>
    using Array = std::array<T, Conf::Limit>;

    using Bitmap = ConfRes::BitmapAllocator<Conf::Limit>;

    Bitmap allocationBitmap;
    Array<Device::VAOHandle> deviceHandle;

    Array<Conf::SubMeshArray> subMesh;
    Array<Conf::SubMeshMaterialArray> materialHandle;

    Array<Conf::VAOBuffers> vaoBuffer;
    Array<HandleType::Generation_t> generations;

    Array<MeshData> meshData;

    std::unordered_map<std::string, HandleType> loadedMeshes; //temporary solution
    ResourceManager *resourceManager = nullptr;
};

//static_assert((sizeof(MaterialManager) % 16) == 0, "Invalid size!");

} //namespace MoonGlare::Renderer::Resources 
