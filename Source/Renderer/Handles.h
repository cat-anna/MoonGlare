#pragma once

namespace MoonGlare::Renderer {

struct TextureResourceHandle {
    using Index_t = uint16_t;
    Index_t m_Index;
    Index_t m_TmpGuard;
    static constexpr Index_t GuardValue = 0xFADE;

    void Reset() {
        memset(this, 0, sizeof(*this));
    }

    operator bool () const {
        return m_TmpGuard == GuardValue;
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
    //operator bool() const {
    //	return m_TmpGuard == GuardValue;
    //}
};
static_assert(std::is_pod<VAOResourceHandle>::value, "Must be pod type!");

//-----------------------------------------------------------------------------

struct ShaderResourceHandleBase {
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
static_assert(std::is_pod<ShaderResourceHandleBase>::value, "Must be pod type!");

template<typename Descirptor>
struct ShaderResourceHandle : public ShaderResourceHandleBase {};

//-----------------------------------------------------------------------------

struct MaterialResourceHandle {
    using Index_t = uint16_t;
    static constexpr Index_t GuardValue = 0x2159;
    Index_t m_Index;
    Index_t m_TmpGuard;

    void Reset() {
        memset(this, 0, sizeof(*this));
    }
    operator bool() const {
        return m_TmpGuard == GuardValue;
    }
};
static_assert(std::is_pod<MaterialResourceHandle>::value, "Must be pod type!");

//-----------------------------------------------------------------------------

struct ResourceHandleBase {
    using Index_t = uint16_t;
    using Generation_t = uint16_t;

    Index_t index;
    Generation_t generation;
};

static_assert(std::is_pod_v<ResourceHandleBase>);
static_assert(sizeof(ResourceHandleBase) == sizeof(void*));

template<typename DeviceHandle_t>
struct HandleTemplate : public ResourceHandleBase {
    using DeviceHandle = DeviceHandle_t;

    static_assert(std::is_pod_v<DeviceHandle>);
    static_assert(sizeof(DeviceHandle) == sizeof(void*));

    DeviceHandle* deviceHandle;
};

//template<typename H>
//static_assert(std::is_pod_v<HandleTemplate<H>>);
//template<typename H>
//static_assert(sizeof(HandleTemplate<H>) == 2 * sizeof(void*));

//-----------------------------------------------------------------------------

using MeshResourceHandle = HandleTemplate<Device::VAOHandle>;

} //namespace MoonGlare::Renderer
