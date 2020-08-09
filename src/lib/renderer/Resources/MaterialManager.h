#pragma once

#include "../nfRenderer.h"
#include "../Configuration.Renderer.h"

#include "../Material.h"

namespace MoonGlare::Renderer::Resources {

class MaterialManager final {
    using ThisClass = MaterialManager;
    using Conf = Configuration::Material;
    using ConfRes = Configuration::Resources;
public:
    MaterialManager(ResourceManager* Owner);
    ~MaterialManager();

    MaterialResourceHandle LoadMaterial(const std::string &uri);
    MaterialResourceHandle CreateMaterial(const std::string &uri, const MaterialTemplate &matTemplate);
    void ApplyTemplate(MaterialResourceHandle handle, const MaterialTemplate &matTemplate);
    Material* GetMaterial(MaterialResourceHandle h);

    //void Release(MaterialResourceHandle h/*, bool ReleaseMaps = false*/); //ReleaseMaps release textures in material

    bool IsHandleValid(MaterialResourceHandle &h) const;

    void ReloadMaterials();
private:
    template<typename T>
    using Array = std::array<T, Conf::Limit>;
    using Bitmap = ConfRes::BitmapAllocator<Conf::Limit>;

    Array<TextureResourceHandle::Generation_t> generations;
    Bitmap m_AllocationBitmap;
    Array<Material> materials;

    std::unordered_map<std::string, MaterialResourceHandle> loadedMaterials; //TODO: temporary solution

    ResourceManager *m_ResourceManager = nullptr;
    iAsyncLoader *asyncLoader = nullptr;

    enum class AllocateResult { Failure, Success, CacheHit, };
    AllocateResult Allocate(MaterialResourceHandle &hout, const std::string &uri);
    AllocateResult Allocate(MaterialResourceHandle &hout);

    void ReloadMaterial(const std::string& uri, MoonGlare::Renderer::MaterialResourceHandle h);
};

//static_assert((sizeof(MaterialManager) % 16) == 0, "Invalid size!");

} //namespace MoonGlare::Renderer::Resources 
