#include "../nfRenderer.h"
#include "../Configuration.Renderer.h"
#include "../Material.h"

#include "MaterialBuilder.h"
#include "ResourceManager.h"
#include "MaterialManager.h"

namespace MoonGlare::Renderer::Resources {

bool MaterialBuilder::SetDiffuseMap(const std::string &URI) {
	return m_Manager->GetResourceManager()->GetTextureResource().LoadTexture(m_MaterialPtr->m_DiffuseMap, URI);
}

bool MaterialBuilder::SetDiffuseMap(const std::string & URI, Configuration::TextureLoad loadcfg) {
	return m_Manager->GetResourceManager()->GetTextureResource().LoadTexture(m_MaterialPtr->m_DiffuseMap, URI, loadcfg);
}

} //namespace MoonGlare::Renderer::Resources 
