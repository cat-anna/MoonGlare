/*
  * Generated by cppsrc.sh
  * On 2017-02-13 22:45:58,54
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#include "ResourceManager.h"
#include "../Renderer.h"

namespace MoonGlare::Renderer::Resources {

bool ResourceManager::Initialize(RendererFacade *Renderer, AssetLoader* Assets) {
	RendererAssert(Renderer);
	RendererAssert(Assets);

	m_RendererFacade = Renderer;
	m_AssetLoader = Assets;

	auto conf = m_RendererFacade->GetConfiguration();

	m_TextureResource.Initialize(this, m_AssetLoader->GetTextureLoader());

	if (!m_VAOResource.Initialize(this)) {
		AddLogf(Error, "VAOResource initialization failed!");
		return true;
	}

	if (!m_ShaderResource.Initialize(this, m_AssetLoader->GetShaderCodeLoader())) {
		AddLogf(Error, "ShaderResource initialization failed!");
		return true;
	}

	m_MaterialManager.Initialize(this);

	return true;
}

bool ResourceManager::Finalize() {
	m_MaterialManager.Finalize();

	if (!m_ShaderResource.Finalize()) {
		AddLogf(Error, "ShaderResource finalization failed!");
	}

	if (!m_VAOResource.Finalize()) {
		AddLogf(Error, "VAOResource finalization failed!");
	}

	m_TextureResource.Finalize();

	return true;
}

const Configuration::RuntimeConfiguration * ResourceManager::GetConfiguration() const {
	return m_RendererFacade->GetConfiguration();
}

} //namespace MoonGlare::Renderer::Resources 
