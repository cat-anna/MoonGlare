/*
  * Generated by cppsrc.sh
  * On 2017-02-04 20:56:37,48
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include "AssetManager.h"

#include "FileSystem.h"
#include "Shader/Loader.h"

#include <AssetSettings.x2c.h>

namespace MoonGlare::Asset {

AssetManager::AssetManager() {
}

AssetManager::~AssetManager() {
}

bool AssetManager::Initialize(const x2c::Settings::AssetSettings_t& Configuration) {

	m_FileSystem = std::make_unique<FileSystem>();
	if (!m_FileSystem->Initialize()) {
		AddLogf(Error, "FileSystem initialization failed!");
		m_FileSystem.reset();
		return false;
	}

	m_ShaderLoader = std::make_unique<Shader::Loader>(m_FileSystem.get());
	if (!m_ShaderLoader->Initialize()) {
		AddLogf(Error, "ShaderLoader initialization failed!");
		m_ShaderLoader.reset();
		return false;
	}

	return true;
}

bool AssetManager::Finalize() {
	if (m_ShaderLoader) {
		if(!m_ShaderLoader->Finalize())
			AddLogf(Error, "ShaderLoader finalization failed!");
		m_ShaderLoader.reset();
	}

	if (m_FileSystem) {
		if(!m_FileSystem->Finalize())
			AddLogf(Error, "FileSystem finalization failed!");
		m_FileSystem.reset();
	}
	return true;
}

} //namespace MoonGlare::Asset
