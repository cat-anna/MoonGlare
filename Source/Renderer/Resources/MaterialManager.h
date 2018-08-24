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

    ResourceManager* GetResourceManager() { return m_ResourceManager; }

    MaterialResourceHandle LoadMaterial(const std::string &uri);
    MaterialResourceHandle CreateMaterial(const std::string &uri, const MaterialTemplate &matTemplate);
    void ApplyTemplate(MaterialResourceHandle handle, const MaterialTemplate &matTemplate);
    Material* GetMaterial(MaterialResourceHandle h);

    bool Allocate(MaterialResourceHandle &hout, const std::string &uri);
    bool Allocate(MaterialResourceHandle &hout);
    //void Release(MaterialResourceHandle h/*, bool ReleaseMaps = false*/); //ReleaseMaps release textures in material

    bool IsHandleValid(MaterialResourceHandle &h) const;
private: 
    template<typename T>
    using Array = std::array<T, Conf::Limit>;
    using Bitmap = ConfRes::BitmapAllocator<Conf::Limit>;

    Array<TextureResourceHandle::Generation_t> generations;
    Bitmap m_AllocationBitmap;
    Array<Material> materials;

    std::unordered_map<std::string, MaterialResourceHandle> loadedMaterials; //temporary solution

    ResourceManager *m_ResourceManager = nullptr;
    iAsyncLoader *asyncLoader;
};

static_assert((sizeof(MaterialManager) % 16) == 0, "Invalid size!");

} //namespace MoonGlare::Renderer::Resources 
