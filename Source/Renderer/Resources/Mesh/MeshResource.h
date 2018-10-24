#pragma once

#include "../../nfRenderer.h"
#include "../../Configuration.Renderer.h"
#include "../../Configuration/Mesh.h"

#include "../../Mesh.h"

namespace MoonGlare::Renderer::Resources {

class MeshManager final {
    using ThisClass = MeshManager;
    using Conf = Configuration::Mesh;
    using ConfRes = Configuration::Resources;
public:
    MeshManager(ResourceManager* Owner);
    ~MeshManager();

    ResourceManager* GetResourceManager() { return resourceManager; }

    MeshResourceHandle Allocate();
    void Release(MeshResourceHandle hin);
                                    
    MeshResourceHandle LoadMesh(const std::string &uri);

    MeshResourceHandle CreateMesh(MeshSource source, const std::string &uri = "");
    void ApplyMeshSource(MeshResourceHandle h, MeshSource source);
    void ApplyMeshSource(MeshResourceHandle h, MeshData source, std::unique_ptr<char[]> sourceMemory);

    bool IsHandleValid(MeshResourceHandle &h) const;

    const Mesh* GetMesh(MeshResourceHandle h) {
        if (!IsHandleValid(h)) {
            return nullptr;
        }
        return &mesh[h.index];
    }
    const MeshData* GetMeshData(MeshResourceHandle h) const {
        if (!IsHandleValid(h)) {
            return nullptr;
        }
        return &meshData[h.index];
    }

    void SaveMeshObj(MeshResourceHandle h, std::string outFile = "");
    void SaveMeshBin(MeshResourceHandle h, std::string outFile = "");
private:
    template<typename T>
    using Array = std::array<T, Conf::Limit>;

    using Bitmap = ConfRes::BitmapAllocator<Conf::Limit>;

    struct MeshFlags {
        //bool meshCommited : 1;
    };
    static_assert(sizeof(MeshFlags) == 1);

    Bitmap allocationBitmap;
    Array<Device::VAOHandle> deviceHandle;
    //Array<MeshFlags> meshFlags;

    Array<Mesh> mesh;

    Array<Conf::VAOBuffers> vaoBuffer;
    Array<MeshResourceHandle::Generation_t> generations;

    Array<MeshData> meshData;
    Array<std::unique_ptr<char[]>> meshDataMemory;

    std::unordered_map<std::string, MeshResourceHandle> loadedMeshes; //temporary solution
    ResourceManager *resourceManager = nullptr;
    iAsyncLoader *asyncLoader = nullptr;

    std::pair<MeshResourceHandle, bool> Allocate(const std::string &uri);
};

//static_assert((sizeof(MaterialManager) % 16) == 0, "Invalid size!");

} //namespace MoonGlare::Renderer::Resources 
