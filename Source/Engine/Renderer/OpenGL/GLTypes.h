#pragma once
#ifndef GLTYPES_H_
#define GLTYPES_H_

namespace MoonGlare {
namespace Renderer {
namespace OpenGL {

using CommandFunction = void(*)(void *DataPtr);

using ShaderHandle = GLuint;
using ShadeUniformLocation = GLint;

using TextureHandle = GLuint;

using VAOHandle = GLuint;

template<typename CMDARG>
struct CommandTemplate {
	static size_t ArgumentSize() { return sizeof(CMDARG); }
	static CommandFunction GetFunction() { return reinterpret_cast<CommandFunction>(&CMDARG::Execute); }
	using Argument = CMDARG;
};

} //namespace OpenGL
} //namespace Renderer 
} //namespace MoonGlare 

#endif // NFRENDERER_H_
