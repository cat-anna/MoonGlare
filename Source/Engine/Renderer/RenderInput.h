#pragma once

#include "Light.h"
#include <Renderer/VirtualCamera.h>

#include <Source/Renderer/Commands/CommandQueue.h>
#include <Source/Renderer/Commands/CommandQueueLayers.h>

#include "Renderer/RendererConfiguration.h"

namespace MoonGlare {
namespace Renderer {

struct RenderInput {
	Graphic::Dereferred::DefferedSink *m_DefferedSink = nullptr;

	VirtualCamera m_Camera;

	using CommandQueueTable = Renderer::Commands::CommandQueueLayers<::MoonGlare::Configuration::Renderer::CommandQueueID>;
	CommandQueueTable m_CommandQueues;

	void Initialize(const math::fvec2 &ScreenSize) {
		m_Camera.SetDefaultPerspective(ScreenSize);
		m_CommandQueues.MemZero();
	}
	void Clear() {
		m_CommandQueues.ClearAllocation();
	}
};

} //namespace Renderer 
} //namespace MoonGlare 
