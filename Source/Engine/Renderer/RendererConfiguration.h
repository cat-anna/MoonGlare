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

	union CommandKey {
		uint16_t m_UIntValue;
		struct {
		//	uint32_t m_Ptr;
			uint16_t m_Order;
		} m_Details;
	};

	static_assert(sizeof(CommandKey) == sizeof(CommandKey::m_UIntValue), "CommandKey has invalid size!");
};

} //namespace Configuration 

namespace Renderer {
	using RendererConf = Configuration::Renderer;
}

} //namespace MoonGlare 

#endif // RENDERERCONFIGURATION_H_