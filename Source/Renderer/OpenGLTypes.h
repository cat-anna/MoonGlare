#pragma once

namespace MoonGlare::Renderer {

struct TextureResourceHandle {
	using Index_t = uint16_t;
	Index_t m_Index;
	Index_t m_TmpGuard;

	void Reset() {
		memset(this, 0, sizeof(*this));
	}
};
static_assert(std::is_pod<TextureResourceHandle>::value, "Must be pod type!");

struct VAOResourceHandle {
	using Index_t = uint16_t;
	Index_t m_Index;
	Index_t m_TmpGuard;

	void Reset() {
		memset(this, 0, sizeof(*this));
	}
};
static_assert(std::is_pod<VAOResourceHandle>::value, "Must be pod type!");

using TextureHandle = GLuint;
static constexpr TextureHandle InvalidTextureHandle = static_cast<TextureHandle>(0);

using VAOHandle = GLuint;
static constexpr VAOHandle InvalidVAOHandle = static_cast<VAOHandle>(0);

using BufferHandle = GLuint;
static constexpr BufferHandle InvalidBufferHandle = static_cast<BufferHandle>(0);

using FramebufferHandle = GLuint;
static constexpr FramebufferHandle InvalidFramebufferHandle = static_cast<FramebufferHandle>(0);

using ShaderHandle = GLuint;
using ShadeUniformLocation = GLint;

//---------------------------------------------------------------------------------------

template<typename T> struct GLTypeInfo {
	static_assert(std::is_same<int, int>::value, "Unknown opengl type!");
};

template<typename T> struct GLTypeInfoCommon {
	using Type = T;
	static constexpr unsigned TypeSize = sizeof(T);
};

template<> struct GLTypeInfo<GLfloat> : public GLTypeInfoCommon<GLfloat> {
	static constexpr GLenum TypeId = GL_FLOAT;
};
template<> struct GLTypeInfo<GLint> : public GLTypeInfoCommon<GLint> {
	static constexpr GLenum TypeId = GL_INT;
};
template<> struct GLTypeInfo<GLuint> : public GLTypeInfoCommon<GLuint> {
	static constexpr GLenum TypeId = GL_UNSIGNED_INT;
};
template<> struct GLTypeInfo<GLushort> : public GLTypeInfoCommon<GLushort> {
	static constexpr GLenum TypeId = GL_UNSIGNED_SHORT;
};
template<> struct GLTypeInfo<GLubyte> : public GLTypeInfoCommon<GLubyte> {
	static constexpr GLenum TypeId = GL_UNSIGNED_BYTE;
};

template<typename T>
constexpr GLenum TypeId = GLTypeInfo<T>::TypeId;

} //namespace MoonGlare::Renderer
