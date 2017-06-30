#pragma once

#include "../nfRenderer.h"
#include "../Configuration.Renderer.h"

#include "../Material.h"

#ifdef NEED_MATERIAL_BUILDER
#include "MaterialBuilder.h"
#endif

namespace MoonGlare::Renderer::Resources {

class alignas(16) MaterialManager final 
    //: iAbstractResource
{
	using ThisClass = MaterialManager;
	using Conf = Configuration::Material;
	using ConfRes = Configuration::Resources;
public:
    MaterialManager(ResourceManager* Owner);
    ~MaterialManager();

	ResourceManager* GetResourceManager() { return m_ResourceManager; }

    bool LoadMaterial(const std::string &uri, MaterialResourceHandle &h);
	Material* GetMaterial(MaterialResourceHandle h);

    bool Allocate(MaterialResourceHandle &hout, const std::string &uri);
    bool Allocate(MaterialResourceHandle &hout);
	void Release(MaterialResourceHandle h/*, bool ReleaseMaps = false*/); //ReleaseMaps release textures in material

    bool IsHandleValid(MaterialResourceHandle &h) const;

#ifdef NEED_MATERIAL_BUILDER
    MaterialBuilder GetMaterialBuilder(MaterialResourceHandle &h, bool AllowAllocation = false) {
		if (AllowAllocation && !IsHandleValid(h)) {
			if (!Allocate(h)) {
				RendererAssert(false);//TODO:
			}
		}

		return MaterialBuilder {
			&m_Materials[h.index],
			this,
		};
	}
#endif

private: 
	template<typename T>
	using Array = std::array<T, Conf::Limit>;
	using Bitmap = ConfRes::BitmapAllocator<Conf::Limit>;

    Array<TextureResourceHandle::Generation_t> generations;
    Bitmap m_AllocationBitmap;
	Array<Material> m_Materials;

    std::unordered_map<std::string, MaterialResourceHandle> loadedMaterials; //temporary solution

	ResourceManager *m_ResourceManager = nullptr;
	void* padding;
};

static_assert((sizeof(MaterialManager) % 16) == 0, "Invalid size!");

} //namespace MoonGlare::Renderer::Resources 
