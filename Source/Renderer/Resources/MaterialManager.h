#pragma once

#include "../nfRenderer.h"
#include "../Configuration.Renderer.h"

#include "../Material.h"

namespace MoonGlare::Renderer::Resources {

class 
	alignas(16) 
	MaterialManager final {
	using ThisClass = MaterialManager;
	using Conf = Configuration::Material;
	using ConfRes = Configuration::Resources;
public:
	void Initialize(ResourceManager* Owner);
	void Finalize();

	Material* GetMaterial(MaterialResourceHandle out);

	bool Allocate(MaterialResourceHandle &out, Material ** materialptr = nullptr);
	void Release(MaterialResourceHandle &h/*, bool ReleaseMaps = false*/); //ReleaseMaps release textures in material

	//bool Allocate(Frame *frame, MaterialResourceHandle &out);

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
