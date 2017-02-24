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

//-----------------------------------------------------------------------------

struct VAOResourceHandle {
	using Index_t = uint16_t;
	Index_t m_Index;
	Index_t m_TmpGuard;

	void Reset() {
		memset(this, 0, sizeof(*this));
	}
};
static_assert(std::is_pod<VAOResourceHandle>::value, "Must be pod type!");

//-----------------------------------------------------------------------------

struct ShaderResourceHandle {
	using Index_t = uint16_t;
	static constexpr Index_t GuardValue = 0xae89;
	Index_t m_TmpGuard;
	Index_t m_Index;

	void Reset() {
		memset(this, 0, sizeof(*this));
	}
	
	operator bool() const {
		return m_TmpGuard == GuardValue;
	}
};
static_assert(std::is_pod<ShaderResourceHandle>::value, "Must be pod type!");

} //namespace MoonGlare::Renderer
