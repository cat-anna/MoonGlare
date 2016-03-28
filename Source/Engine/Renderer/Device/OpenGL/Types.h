#pragma once
namespace MoonGlare {
namespace Renderer {
namespace Devices {
namespace OpenGL {

enum class TextureType : GLuint {
	RGB = GL_RGB,
	BGR = GL_BGR,
	BGRA = GL_BGRA,
	RGBA = GL_RGBA,
//	LuminanceAlpha = GL_LUMINANCE_ALPHA,
};

enum class ElementMode : GLuint {
	Quads = GL_QUADS,
	Triangles = GL_TRIANGLES,
};

using TextureHandle = GLuint;

//enum {
//	fUnsignedChar = GL_UNSIGNED_BYTE,
//	fUnsignedInt = GL_UNSIGNED_INT,
//};

} //namespace OpenGL
} //namespace Devices 
} //namespace Renderer 
} //namespace MoonGlare 
