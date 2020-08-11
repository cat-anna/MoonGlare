#pragma once

#include <cstdint>
#include <gl/glew.h>
#include <type_traits>

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

enum class PixelFormat : uint16_t {
    Red = GL_RED,
    RedGreen = GL_RG,
    RedGreen8 = GL_RG8,
    RGB8 = GL_RGB,
    RGBA8 = GL_RGBA,

    SRGB8 = GL_SRGB,
    SRGBA8 = GL_SRGB8_ALPHA8,
};

inline unsigned BppFromPixelFormat(PixelFormat pf) {
    switch (pf) {
    case PixelFormat::Red:
        return 8;
    case PixelFormat::RedGreen8:
    case PixelFormat::RedGreen:
        return 16;
    case PixelFormat::RGB8:
    case PixelFormat::SRGB8:
        return 24;
    case PixelFormat::RGBA8:
    case PixelFormat::SRGBA8:
        return 32;
    default:
        return 0;
    }
}
enum class ValueFormat : uint16_t {
    UnsignedByte = GL_UNSIGNED_BYTE, // this constants should not be used here!!!
    Float = GL_FLOAT,
};

//-----------------------------------------------------------------------------

template <typename T> struct TypeInfo { static_assert(std::is_same<int, int>::value, "Unknown opengl type!"); };

template <typename T> struct TypeInfoCommon {
    using Type = T;
    static constexpr unsigned TypeSize = sizeof(T);
};

template <> struct TypeInfo<GLfloat> : public TypeInfoCommon<GLfloat> { static constexpr GLenum TypeId = GL_FLOAT; };
template <> struct TypeInfo<GLint> : public TypeInfoCommon<GLint> { static constexpr GLenum TypeId = GL_INT; };
template <> struct TypeInfo<GLuint> : public TypeInfoCommon<GLuint> {
    static constexpr GLenum TypeId = GL_UNSIGNED_INT;
};
template <> struct TypeInfo<GLushort> : public TypeInfoCommon<GLushort> {
    static constexpr GLenum TypeId = GL_UNSIGNED_SHORT;
};
template <> struct TypeInfo<GLubyte> : public TypeInfoCommon<GLubyte> {
    static constexpr GLenum TypeId = GL_UNSIGNED_BYTE;
};

template <typename T> constexpr GLenum TypeId = TypeInfo<T>::TypeId;

} // namespace MoonGlare::Renderer::Device