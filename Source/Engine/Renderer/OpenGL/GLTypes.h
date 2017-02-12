#pragma once

#include <Source/Renderer/Commands/CommandQueueBase.h>

namespace MoonGlare {
namespace Renderer {

using ShaderHandle = GLuint;
using ShadeUniformLocation = GLint;

using TextureHandle = GLuint;

using VAOHandle = GLuint;

template<typename CMDARG>
struct CommandTemplate {
	using Argument = CMDARG;
	static size_t ArgumentSize() {
		return sizeof(Argument);
	}
	static CommandFunction GetFunction() {
		return reinterpret_cast<CommandFunction>(&Argument::Execute);
	}
};

} //namespace Renderer 
} //namespace MoonGlare 

