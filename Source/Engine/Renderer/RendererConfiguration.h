#pragma once
#ifndef RENDERERCONFIGURATION_H_
#define RENDERERCONFIGURATION_H_

namespace MoonGlare {
namespace Configuration {

struct Renderer {
	struct CommandQueue {
		enum {
			ArgumentMemoryBuffer	= 1024 * 1024, //1mb
			CommandLimit			= 4096,
			BytesPerCommand			= ArgumentMemoryBuffer / CommandLimit,
		};
	};
};

} //namespace Configuration 

namespace Renderer {
	using RendererConf = Configuration::Renderer;
}

} //namespace MoonGlare 

#endif // RENDERERCONFIGURATION_H_
