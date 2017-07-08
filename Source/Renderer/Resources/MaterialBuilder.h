#pragma once

#include "../nfRenderer.h"
#include "../Configuration.Renderer.h"
#include "../Material.h"

namespace MoonGlare::Renderer::Resources {

class MaterialBuilder final {
	using Conf = Configuration::Material;
	using ConfRes = Configuration::Resources;
public:
	void SetDiffuseColor(const emath::fvec4 &col) { m_MaterialPtr->m_DiffuseColor = col; }
	bool SetDiffuseMap(const std::string &URI, bool needsize = false);
    bool SetDiffuseMap(const std::string &URI, Configuration::TextureLoad loadcfg);
    bool SetDiffuseMap();

	//template<typename T>
	//void SetDiffuseMapPixels(Commands::CommandQueue &q, const T* Pixels, const emath::usvec2 &size,
	//	Configuration::TextureLoad loadcfg, Device::PixelType pxtype, Commands::CommandKey key = Commands::CommandKey()) {
	//}
	//(expanded_data, emath::usvec2(width, height), tload, Renderer::Device::PixelType::LuminanceAlpha);

	Material* m_MaterialPtr;
	MaterialManager *m_Manager;
};

//static_assert((sizeof(MaterialManager) % 16) == 0, "Invalid size!");
static_assert(std::is_trivial<MaterialBuilder>::value, "must be trivial!");

} //namespace MoonGlare::Renderer::Resources 
