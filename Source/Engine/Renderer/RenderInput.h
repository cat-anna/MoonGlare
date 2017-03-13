#pragma once

#include "Light.h"
#include <Renderer/VirtualCamera.h>

namespace MoonGlare {
namespace Renderer {

struct RenderInput {
	Graphic::Dereferred::DefferedSink *m_DefferedSink = nullptr;
	VirtualCamera m_Camera;
};

} //namespace Renderer 
} //namespace MoonGlare 
