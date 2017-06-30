#pragma once

#include "Handles.h"

namespace MoonGlare::Renderer {

struct Material {
	emath::fvec4 m_DiffuseColor;
	TextureResourceHandle m_DiffuseMap;

	//bool m_HasNormalMap;
	//TextureResourceHandle m_NormalMap;

	void Reset() {
		m_DiffuseColor = emath::fvec4(1);
		//m_DiffuseMap.Zero();
	}
};

//static_assert((sizeof(Material) % 16) == 0, "Invalid size!");
//static_assert(std::is_trivial<Material>::value, "Must be trivial");

} //namespace MoonGlare::Renderer
