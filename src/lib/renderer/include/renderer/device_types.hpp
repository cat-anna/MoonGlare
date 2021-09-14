#pragma once

#include <cstdint>
#include <glad/glad.h>
#include <type_traits>

namespace MoonGlare::Renderer::Device {

using ShaderHandle = GLuint;
constexpr ShaderHandle kInvalidShaderHandle = 0;
constexpr ShaderHandle InvalidShaderStageHandle = 0;

using ShaderUniformHandle = GLint;
constexpr ShaderUniformHandle kInvalidShaderUniformHandle = -1;

using BufferHandle = GLuint;
constexpr BufferHandle kInvalidBufferHandle = 0;

using VaoHandle = GLuint;
constexpr VaoHandle kInvalidVaoHandle = 0;

using TextureHandle = GLuint;
constexpr TextureHandle kInvalidTextureHandle = 0;

using FrameBufferHandle = GLuint;
static constexpr FrameBufferHandle kInvalidFrameBufferHandle = 0;

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

} // namespace MoonGlare::Renderer::Device
