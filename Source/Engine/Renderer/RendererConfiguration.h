#pragma once

namespace MoonGlare {
namespace Configuration {

namespace Renderer {
	enum class CommandQueueID {
		GUI,
		DefferedShadow,
		DefferedGeometry,

		MaxValue,
	};

	using CommandKey = ::MoonGlare::Renderer::Commands::CommandKey;
};

} //namespace Configuration 
} //namespace MoonGlare 
