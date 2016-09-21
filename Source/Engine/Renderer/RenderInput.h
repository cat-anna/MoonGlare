#pragma once

#include <libSpace/src/Container/StaticVector.h>

#include "Light.h"
#include "VirtualCamera.h"

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

struct D2AnimRenderJob {
	math::mat4 m_Matrix;
//	math::vec3 m_BaseColor;
	GUI::SharedAnimation m_Animation;
//	Graphic::VAO *m_VAO;
	unsigned m_Frame;
};

struct RenderInput {

	std::vector<std::pair<math::mat4, DataClasses::ModelPtr>> m_RenderList;
	std::vector<D2AnimRenderJob> m_D2AnimRenderList;

	//TODO: 1024?
	Space::Container::StaticVector<Light::PointLight, 128> m_PointLights;
	Space::Container::StaticVector<Light::SpotLight, 128> m_SpotLights;
	Space::Container::StaticVector<Light::DirectionalLight, 128> m_DirectionalLights;

	VirtualCamera m_Camera;

	//shall be used for gui debug
	std::vector<math::vec3> m_2DPoints; //rendered as lines
	std::vector<math::vec3> m_2DColors; //one per line, shall be 2x smaller

	bool Initialize(const math::fvec2 &ScreenSize) {
		m_RenderList.reserve(2048); 
		m_D2AnimRenderList.reserve(2048);
		m_PointLights.ClearAllocation();
		m_SpotLights.ClearAllocation();
		m_DirectionalLights.ClearAllocation();
		m_Camera.SetDefaultPerspective(ScreenSize);
		m_2DPoints.reserve(1 << 14);
		m_2DColors.reserve(1 << 14);
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
		m_D2AnimRenderList.clear();
		m_2DPoints.clear();
		m_2DColors.clear();
	}
};

} //namespace Renderer 
} //namespace MoonGlare 
