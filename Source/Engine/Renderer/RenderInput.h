#pragma once

#include "Light.h"
#include <Renderer/VirtualCamera.h>

#include <Source/Renderer/Commands/CommandQueue.h>
#include <Source/Renderer/Commands/CommandQueueLayers.h>

#include "Renderer/RendererConfiguration.h"

namespace MoonGlare {
namespace GUI {
	class Animation;
	using SharedAnimation = std::shared_ptr < Animation >;
}
}

namespace MoonGlare {
namespace Renderer {

struct RenderInput {
	Graphic::Dereferred::DefferedSink *m_DefferedSink = nullptr;

	Space::Container::StaticVector<Light::PointLight, 128> m_PointLights;
	Space::Container::StaticVector<Light::SpotLight, 128> m_SpotLights;
	Space::Container::StaticVector<Light::DirectionalLight, 128> m_DirectionalLights;

	VirtualCamera m_Camera;

	using CommandQueueTable = Renderer::Commands::CommandQueueLayers<::MoonGlare::Configuration::Renderer::CommandQueueID>;
	CommandQueueTable m_CommandQueues;

	bool Initialize(const math::fvec2 &ScreenSize) {
		m_PointLights.ClearAllocation();
		m_SpotLights.ClearAllocation();
		m_DirectionalLights.ClearAllocation();
		m_Camera.SetDefaultPerspective(ScreenSize);
		m_CommandQueues.MemZero();
		return true;
	}
	void Clear() {
		m_PointLights.ClearAllocation();
		m_SpotLights.ClearAllocation();
		m_DirectionalLights.ClearAllocation();
		m_CommandQueues.ClearAllocation();
	}
};

} //namespace Renderer 
} //namespace MoonGlare 
