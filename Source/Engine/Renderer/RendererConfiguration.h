#pragma once

namespace MoonGlare {
namespace Configuration {

namespace Renderer {
	enum class CommandQueueID {
		PrepareFrame,
		GUI,
		DefferedShadow,
		DefferedGeometry,

		MaxValue,
	};

	using CommandKey = ::MoonGlare::Renderer::Commands::CommandKey;
};

} //namespace Configuration 

namespace Renderer {
	namespace RendererConf = ::MoonGlare::Configuration::Renderer;
}

} //namespace MoonGlare 
