/*
* Generated by cppsrc.sh
* On 2017-02-10 21:54:06,96
* by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include <Source/Assets/AssetLoaderInterface.h>

namespace MoonGlare::Renderer::Device {

//-----------------------------------------------------------------------------

using TextureHandle = GLuint;
static constexpr TextureHandle InvalidTextureHandle = static_cast<TextureHandle>(0);

using VAOHandle = GLuint;
static constexpr VAOHandle InvalidVAOHandle = static_cast<VAOHandle>(0);

using BufferHandle = GLuint;
static constexpr BufferHandle InvalidBufferHandle = static_cast<BufferHandle>(0);

using FramebufferHandle = GLuint;
static constexpr FramebufferHandle InvalidFramebufferHandle = static_cast<FramebufferHandle>(0);

using ShaderHandle = GLuint;
static constexpr ShaderHandle InvalidShaderHandle = static_cast<ShaderHandle>(0);

using ShaderUniformHandle = GLint;
static constexpr ShaderUniformHandle InvalidShaderUniformHandle = static_cast<ShaderUniformHandle>(-1);

using ShaderStageHandle = GLuint;
static constexpr ShaderStageHandle InvalidShaderStageHandle = static_cast<ShaderStageHandle>(0);

//-----------------------------------------------------------------------------

using PixelFormat = Asset::TextureLoader::PixelFormat;
using ValueFormat = Asset::TextureLoader::ValueFormat;

//-----------------------------------------------------------------------------

template<typename T> struct TypeInfo {
    static_assert(std::is_same<int, int>::value, "Unknown opengl type!");
};

template<typename T> struct TypeInfoCommon {
    using Type = T;
    static constexpr unsigned TypeSize = sizeof(T);
};

template<> struct TypeInfo<GLfloat> : public TypeInfoCommon<GLfloat> {
    static constexpr GLenum TypeId = GL_FLOAT;
};
template<> struct TypeInfo<GLint> : public TypeInfoCommon<GLint> {
    static constexpr GLenum TypeId = GL_INT;
};
template<> struct TypeInfo<GLuint> : public TypeInfoCommon<GLuint> {
    static constexpr GLenum TypeId = GL_UNSIGNED_INT;
};
template<> struct TypeInfo<GLushort> : public TypeInfoCommon<GLushort> {
    static constexpr GLenum TypeId = GL_UNSIGNED_SHORT;
};
template<> struct TypeInfo<GLubyte> : public TypeInfoCommon<GLubyte> {
    static constexpr GLenum TypeId = GL_UNSIGNED_BYTE;
};

template<typename T>
constexpr GLenum TypeId = TypeInfo<T>::TypeId;

} //namespace MoonGlare::Renderer::Device
