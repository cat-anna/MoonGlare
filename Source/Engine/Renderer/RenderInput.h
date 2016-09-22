#pragma once

#include <libSpace/src/Container/StaticVector.h>

#include "Light.h"
#include "VirtualCamera.h"

#include "CommandQueue.h"

namespace MoonGlare {
//namespace Graphic {
//	class VAO;
//}
namespace GUI {
	class Animation;
	using SharedAnimation = std::shared_ptr < Animation >;
}
}

namespace MoonGlare {
namespace Renderer {

enum class CommandQueueID {
	GUI,
	MaxValue,
};

struct SimpleVAORenderJob {
//VAO
//texture
//back color
//matrix

//BaseVertex;
//BaseIndex;
//NumIndices;
//ElementMode;
};

struct RenderInput {

	std::vector<std::pair<math::mat4, DataClasses::ModelPtr>> m_RenderList;

	//TODO: 1024?
	Space::Container::StaticVector<Light::PointLight, 128> m_PointLights;
	Space::Container::StaticVector<Light::SpotLight, 128> m_SpotLights;
	Space::Container::StaticVector<Light::DirectionalLight, 128> m_DirectionalLights;

	VirtualCamera m_Camera;

	//shall be used for gui debug
	std::vector<math::vec3> m_2DPoints; //rendered as lines
	std::vector<math::vec3> m_2DColors; //one per line, shall be 2x smaller

	std::array<CommandQueue, static_cast<size_t>(CommandQueueID::MaxValue)> m_CommandQueues;

	bool Initialize(const math::fvec2 &ScreenSize) {
		m_RenderList.reserve(2048); 
		m_PointLights.ClearAllocation();
		m_SpotLights.ClearAllocation();
		m_DirectionalLights.ClearAllocation();
		m_Camera.SetDefaultPerspective(ScreenSize);
		m_2DPoints.reserve(1 << 14);
		m_2DColors.reserve(1 << 14);
		for (auto &it : m_CommandQueues)
			it.MemZero();
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
		m_2DPoints.clear();
		m_2DColors.clear();
		for (auto &it : m_CommandQueues)
			it.ClearAllocation();
	}
};

} //namespace Renderer 
} //namespace MoonGlare 
