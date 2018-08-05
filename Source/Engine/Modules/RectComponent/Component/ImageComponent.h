#pragma once

#include "nfGUIComponent.h"
#include <Core/Component/TemplateStandardComponent.h>

#include "../GUIShader.h"

namespace MoonGlare::GUI::Component {

using namespace Core::Component;

union ImageComponentEntryFlagsMap {
    struct MapBits_t {
        bool m_Valid : 1;
        bool m_Dirty : 1;
        bool m_Active : 1;
    };
    MapBits_t m_Map;
    uint8_t m_UintValue;

    void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
    void ClearAll() { m_UintValue = 0; }

    static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
};

struct ImageComponentEntry {
    Handle m_SelfHandle;
    Entity m_OwnerEntity;
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

class ImageComponent 
    : public TemplateStandardComponent<ImageComponentEntry, ComponentID::Image> {
public:
    ImageComponent(SubsystemManager *Owner);
    virtual ~ImageComponent();
    virtual bool Initialize() override;
    virtual bool Finalize() override;
    virtual void Step(const Core::MoveConfig &conf) override;
    virtual bool Load(xml_node node, Entity Owner, Handle &hout) override;

    static void RegisterScriptApi(ApiInitializer &root);
protected:
    RectTransformComponent *m_RectTransform;
    Renderer::ShaderResourceHandle<GUIShaderDescriptor> m_ShaderHandle;
};

}
