#pragma once

#include "resource_id.hpp"
#include <cstdint>
#include <glad/glad.h>
#include <type_traits>

namespace MoonGlare::Renderer {

using RenderElementIndexType = uint16_t;
using ResourceHandle = MoonGlare::ResourceHandle;

constexpr ResourceHandle kResourceUnusedMask = 0x0F00000000000000;

constexpr ResourceHandle kResourceDeviceHandleMask = 0x00000000FFFFFFFF;

constexpr ResourceHandle kResourceTypeMask = 0xF000000000000000;
constexpr ResourceHandle kResourceTypeShader = 0x1000000000000000;
constexpr ResourceHandle kResourceTypeTexture = 0x2000000000000000;

//TODO: resource generation is ignored, but bits are reserved
constexpr ResourceHandle kResourceGenerationMask = 0x00FFFFFF00000000;

using ResourceGeneration = uint32_t;

//-----------------------------------------------------------------------------

using TextureHandle = ResourceHandle;
static constexpr TextureHandle kInvalidTextureHandle = static_cast<TextureHandle>(0);

// using VAOHandle = GLuint;
// static constexpr VAOHandle InvalidVAOHandle = static_cast<VAOHandle>(0);

// using BufferHandle = GLuint;
// static constexpr BufferHandle InvalidBufferHandle = static_cast<BufferHandle>(0);

// using FramebufferHandle = GLuint;
// static constexpr FramebufferHandle InvalidFramebufferHandle = static_cast<FramebufferHandle>(0);

using ShaderHandle = ResourceHandle;
static constexpr ShaderHandle kInvalidShaderHandle = static_cast<ShaderHandle>(0);

// using ShaderUniformHandle = GLint;
// static constexpr ShaderUniformHandle InvalidShaderUniformHandle = static_cast<ShaderUniformHandle>(-1);

// using ShaderStageHandle = GLuint;
// static constexpr ShaderStageHandle InvalidShaderStageHandle = static_cast<ShaderStageHandle>(0);

//-----------------------------------------------------------------------------

template <typename T>
struct TypeInfo {
    static_assert(std::is_same<int, int>::value, "Unknown opengl type!");
};

template <typename T>
struct TypeInfoCommon {
    using Type = T;
    static constexpr unsigned TypeSize = sizeof(T);
};

template <>
struct TypeInfo<GLfloat> : public TypeInfoCommon<GLfloat> {
    static constexpr GLenum TypeId = GL_FLOAT;
};
template <>
struct TypeInfo<GLint> : public TypeInfoCommon<GLint> {
    static constexpr GLenum TypeId = GL_INT;
};
template <>
struct TypeInfo<GLuint> : public TypeInfoCommon<GLuint> {
    static constexpr GLenum TypeId = GL_UNSIGNED_INT;
};
template <>
struct TypeInfo<GLushort> : public TypeInfoCommon<GLushort> {
    static constexpr GLenum TypeId = GL_UNSIGNED_SHORT;
};
template <>
struct TypeInfo<GLubyte> : public TypeInfoCommon<GLubyte> {
    static constexpr GLenum TypeId = GL_UNSIGNED_BYTE;
};

template <typename T>
constexpr GLenum TypeId = TypeInfo<std::decay_t<std::remove_cv_t<T>>>::TypeId;

} // namespace MoonGlare::Renderer
