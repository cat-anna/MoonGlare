#pragma once

#include "../component_common.hpp"
#include "math/vector.hpp"
#include "rect_enums.hpp"
#include "resource_id.hpp"
#include <cassert>
#include <cstddef>

namespace MoonGlare::Component::Rect {

struct alignas(16) RectImage : public ComponentBase<RectImage> {
    static constexpr ComponentId kComponentId = 17;
    static constexpr char kComponentName[] = "rect.image";
    static constexpr bool kEditable = true;
    static constexpr bool kSerializable = true;
    static constexpr bool kHasResources = true;

    TextureRuntimeResourceHandle image_handle;
    ShaderRuntimeResourceHandle shader_handle;

    void SetDirty() {}
};

// static_assert((sizeof(Image) % 16) == 0);
// static_assert((offsetof(Image, scale) % 16) == 0);
// static_assert((offsetof(Image, position) % 16) == 0);
// static_assert((offsetof(Image, quaternion) % 16) == 0);

#ifdef WANTS_TYPE_INFO

auto GetTypeInfo(RectImage *) {
    return AttributeMapBuilder<RectImage>::Start(RectImage::kComponentName)
        ->AddField("image_handle", &RectImage::image_handle)
        ->AddField("shader_handle", &RectImage::shader_handle)
        // ->AddField("position", &RectImage::position)
        // ->AddField("quaternion", &RectImage::quaternion)]
        ;
}

#endif

#ifdef WANTS_SERIALIZATION

void to_json(nlohmann::json &j, const RectImage &p) {
    j = {
        {"image_handle", p.image_handle},   //
        {"shader_handle", p.shader_handle}, //
        // {"position", p.position},
        // {"quaternion", p.quaternion},
    };
}
void from_json(const nlohmann::json &j, RectImage &p) {
    j.at("image_handle").get_to(p.image_handle);
    j.at("shader_handle").get_to(p.shader_handle);
    // j.at("position").get_to(p.position);
    // j.at("quaternion").get_to(p.quaternion);
    p.SetDirty();
}

void LoadResources(RectImage &image, iRuntimeResourceLoader &loader) {
    loader.LoadResource(image.image_handle);
    loader.LoadResource(image.shader_handle);
}

#endif

} // namespace MoonGlare::Component::Rect

#if 0

namespace MoonGlare::GUI::Component {

struct ImageComponentEntry {
    Entity m_Owner;
    char padding[3];
    ImageComponentEntryFlagsMap m_Flags;

    float m_Speed;
    float m_Position;
    math::vec4 m_Color;
    DEFINE_COMPONENT_PROPERTY(Speed);
    DEFINE_COMPONENT_PROPERTY(Position);
    DEFINE_COMPONENT_PROPERTY(Color);
    //TODO: FrameCount property
    //TODO: ScaleMode
    //TODO: AnimationTexture

    unsigned m_StartFrame;
    unsigned m_EndFrame;
    math::uvec2 m_FrameCount;

    ImageScaleMode m_ScaleMode;

    math::mat4 m_ImageMatrix;
    MoonGlare::Configuration::RuntimeRevision m_TransformRevision;

    Renderer::MaterialResourceHandle material{};
    Renderer::VAOResourceHandle vaoHandle{};
    bool m_DrawEnabled = false;
    math::vec2 m_FrameSize;

    emath::ivec2 GetFrameIndex() {
        uint32_t index = static_cast<uint32_t>(m_Position);
        if (index > m_EndFrame)
            index = m_EndFrame;
        else
            if (index < m_StartFrame)
                index = m_StartFrame;
        return {
            static_cast<int>(index % m_FrameCount[0]),
            static_cast<int>(index / m_FrameCount[0]),
        };
    }

    void SetDirty() { m_Flags.m_Map.m_Dirty = true; m_TransformRevision = 0; }
    void Reset() {
        m_Flags.ClearAll();
        //m_Animation.reset();
        //material
        //vaoHandle
        m_TransformRevision = 0;
    }

    void Update(float TimeDelta, RectTransformComponentEntry &rectTransform);
    bool Load(const std::string &fileuri, math::uvec2 FrameStripCount, bool Uniform, const emath::fvec2 &ScreenSize);
};
//static_assert((sizeof(RectTransformComponentEntry) % 16) == 0, "RectTransformComponentEntry has invalid size");
//static_assert(std::is_pod<RectTransformComponentEntry>::value, "RectTransformComponentEntry must be pod!");

}

#endif
