#pragma once

#include <libSpace/src/Container/StaticVector.h>

#include "Light.h"

namespace MoonGlare {
namespace Renderer {

struct RenderInput {

	std::vector<std::pair<math::mat4, DataClasses::ModelPtr>> m_RenderList;

	//TODO: 1024?
	Space::Container::StaticVector<Light::PointLight, 128> m_PointLights;
	Space::Container::StaticVector<Light::SpotLight, 128> m_SpotLights;
	Space::Container::StaticVector<Light::DirectionalLight, 128> m_DirectionalLights;

	Graphic::VirtualCamera m_Camera;

	bool Initialize(const math::fvec2 &ScreenSize) {
		m_RenderList.reserve(2048);
		m_PointLights.ClearAllocation();
		m_SpotLights.ClearAllocation();
		m_DirectionalLights.ClearAllocation();
		m_Camera.SetDefaultPerspective(ScreenSize);
		return true;
	}

	void OnBeginFrame(cRenderDevice &dev) {
		dev.Bind(&m_Camera);
	}
	void OnEndFrame() {
		m_PointLights.ClearAllocation();
		m_SpotLights.ClearAllocation();
		m_DirectionalLights.ClearAllocation();
		m_RenderList.clear();
	}
};

} //namespace Renderer 
} //namespace MoonGlare 
