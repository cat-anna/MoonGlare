#pragma once

#include "../nfRenderer.h"
#include "../Configuration.Renderer.h"

#include "../Material.h"

#include "MaterialBuilder.h"

namespace MoonGlare::Renderer::Resources {

class alignas(16) MaterialManager final 
    //: iAbstractResource
{
	using ThisClass = MaterialManager;
	using Conf = Configuration::Material;
	using ConfRes = Configuration::Resources;
public:
	void Initialize(ResourceManager* Owner);
	void Finalize();

	ResourceManager* GetResourceManager() { return m_ResourceManager; }

	Material* GetMaterial(MaterialResourceHandle out);

	bool Allocate(MaterialResourceHandle &out, Material ** materialptr = nullptr);
	void Release(MaterialResourceHandle &h/*, bool ReleaseMaps = false*/); //ReleaseMaps release textures in material

	//bool Allocate(Frame *frame, MaterialResourceHandle &out);

	MaterialBuilder GetMaterialBuilder(MaterialResourceHandle &h, bool AllowAllocation = false) {
		if (AllowAllocation && h.m_TmpGuard != h.GuardValue) {
			if (!Allocate(h)) {
				RendererAssert(false);//TODO:
			}
		}

		RendererAssert(h.m_TmpGuard == h.GuardValue);
		RendererAssert(h.m_Index < Conf::Limit);

		return MaterialBuilder {
			&m_Materials[h.m_Index],
			this,
		};
	}
private: 
	template<typename T>
	using Array = std::array<T, Conf::Limit>;
	using Bitmap = ConfRes::BitmapAllocator<Conf::Limit>;

	Array<Material> m_Materials;
	//Array<Asset::FileHash> m_SourceHash;
	Bitmap m_AllocationBitmap;

	ResourceManager *m_ResourceManager = nullptr;
	void* padding;
};

static_assert((sizeof(MaterialManager) % 16) == 0, "Invalid size!");

} //namespace MoonGlare::Renderer::Resources 
