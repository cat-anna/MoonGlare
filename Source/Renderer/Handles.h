#pragma once

namespace MoonGlare::Renderer {

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

struct ResourceHandleBase {
    using Index_t = uint16_t;
    using Generation_t = uint16_t;

    Index_t index;
    Generation_t generation;
#ifdef X64
    uint32_t padding;
#endif
};

static_assert(std::is_pod_v<ResourceHandleBase>);
//static_assert(sizeof(ResourceHandleBase) == sizeof(void*));

template<typename DeviceHandle_t, typename DiffType = void>
struct HandleTemplate : public ResourceHandleBase {
    using DeviceHandle = DeviceHandle_t;

    DeviceHandle* deviceHandle;

    void Zero() {
        memset(this, 0, sizeof(*this));
    }

    bool operator==(const HandleTemplate &other) const {
        return index == other.index && generation == other.generation;
    }
};

//-----------------------------------------------------------------------------

struct Material;

namespace detail {
struct VAO{};
struct Texture{};
struct Mesh{};
}

using MeshResourceHandle = HandleTemplate<Device::VAOHandle, detail::Mesh>;
using VAOResourceHandle = HandleTemplate<Device::VAOHandle, detail::VAO>;
using TextureResourceHandle = HandleTemplate<Device::TextureHandle, detail::Texture>;

using MaterialResourceHandle = HandleTemplate<Material>;

static_assert(std::is_trivial_v<MeshResourceHandle>);               //TODO: check why pod is failing
static_assert(sizeof(MeshResourceHandle) == 2 * sizeof(void*));


} //namespace MoonGlare::Renderer
