#include "../../nfRenderer.h"
#include "../../Configuration.Renderer.h"
#include "../../Material.h"

#include "MaterialBuilder.h"
#include "../ResourceManager.h"
#include "../MaterialManager.h"

namespace MoonGlare::Renderer::Resources {

bool MaterialBuilder::SetDiffuseMap(const std::string &URI, bool needsize)  {
    return m_Manager->GetResourceManager()->GetTextureResource().LoadTexture(m_MaterialPtr->m_DiffuseMap, URI, Configuration::TextureLoad::Default(), true, needsize);
}

bool MaterialBuilder::SetDiffuseMap(const std::string & URI, Configuration::TextureLoad loadcfg) {
	return m_Manager->GetResourceManager()->GetTextureResource().LoadTexture(m_MaterialPtr->m_DiffuseMap, URI, loadcfg);
}

bool MaterialBuilder::SetDiffuseMap() {
    return m_Manager->GetResourceManager()->GetTextureResource().Allocate(m_MaterialPtr->m_DiffuseMap);
}

} //namespace MoonGlare::Renderer::Resources 
