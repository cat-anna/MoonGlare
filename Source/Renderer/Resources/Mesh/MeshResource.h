#pragma once

#include "../../nfRenderer.h"
#include "../../Configuration.Renderer.h"
#include "../../Configuration/Mesh.h"

#include "../../Mesh.h"

#ifdef NEED_MESH_BUILDER
#include "MeshBuilder.h"
#endif

namespace MoonGlare::Renderer::Resources {

struct MeshData {
    //TODO: switch to std::uptr
    std::vector<glm::fvec3> verticles;
    std::vector<glm::fvec2> UV0;
    std::vector<glm::fvec3> normals;
    std::vector<uint32_t> index;
    emath::fvec3 halfBoundingBox;
    float boundingRadius;

    void UpdateBoundary();
};

class MeshManager final 
    //: iAbstractResource
{
    using ThisClass = MeshManager;
    using Conf = Configuration::Mesh;
    using ConfRes = Configuration::Resources;
public:
    MeshManager(ResourceManager* Owner);
    ~MeshManager();

    ResourceManager* GetResourceManager() { return resourceManager; }

    using HandleType = MeshResourceHandle;

    bool Allocate(HandleType &hout);
    void Release(HandleType hin);

    HandleType Allocate() {
        HandleType h = {};
        Allocate(h);
        return h;
    }
                                    
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
            *this,
            subMesh[h.index],
            //materialHandle[h.index],
            h,
            meshData[h.index],
            q,
            key,
        };
}
#endif

    bool LoadMesh(const std::string &uri, HandleType &hout);

    HandleType LoadMesh(const std::string &uri) {
        HandleType h = {};
        LoadMesh(uri, h);
        return h;
    }

    bool IsHandleValid(HandleType &h) const;

    const Mesh* GetMeshes(HandleType h) {
        if (!IsHandleValid(h) || !meshFlags[h.index].meshCommited) {
            //TODO
            return nullptr;
        }
        return &subMesh[h.index];
    }
    //const MaterialResourceHandle *GetMaterials(HandleType h) {
    //    if (!IsHandleValid(h) || !meshFlags[h.index].meshCommited) {
    //        //TODO
    //        return nullptr;
    //    }
    //    return &materialHandle[h.index];
    //}
    const MeshData *GetMeshData(HandleType h) {
        if (!IsHandleValid(h)) {
            //TODO
            return nullptr;
        }
        return &meshData[h.index];
    }

    void SetMeshData(HandleType h, MeshData data) {
        if (!IsHandleValid(h)) {
            //TODO
            __debugbreak();
            return;
        }
        meshFlags[h.index].meshCommited = false;
        meshData[h.index] = std::move(data);
    }
    void CommitMesh(HandleType h) {
        if (!IsHandleValid(h)) {
            //TODO
            __debugbreak();
            return;
        }
        meshFlags[h.index].meshCommited = true;
    }
private:
    template<typename T>
    using Array = std::array<T, Conf::Limit>;

    using Bitmap = ConfRes::BitmapAllocator<Conf::Limit>;

    struct MeshFlags {
        bool meshCommited : 1;
    };
    static_assert(sizeof(MeshFlags) == 1);

    Bitmap allocationBitmap;
    Array<Device::VAOHandle> deviceHandle;
    Array<MeshFlags> meshFlags;

    Array<Mesh> subMesh;
    //Array<MaterialResourceHandle> materialHandle;

    Array<Conf::VAOBuffers> vaoBuffer;
    Array<HandleType::Generation_t> generations;

    Array<MeshData> meshData;

    std::unordered_map<std::string, HandleType> loadedMeshes; //temporary solution
    ResourceManager *resourceManager = nullptr;
};

//static_assert((sizeof(MaterialManager) % 16) == 0, "Invalid size!");

} //namespace MoonGlare::Renderer::Resources 
