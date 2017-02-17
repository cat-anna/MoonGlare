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


using FramebufferHandle = GLuint;

using ShaderHandle = GLuint;
using ShadeUniformLocation = GLint;

} //namespace MoonGlare::Renderer
