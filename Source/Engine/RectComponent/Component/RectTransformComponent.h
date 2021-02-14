#pragma once

#include <Memory/PackedArrayIndexTree.h>

#include <Foundation/Component/EntityEvents.h>

#include <Core/Component/TemplateStandardComponent.h>
#include <Core/Scripts/ScriptComponent.h>

#include <Renderer/VirtualCamera.h>

#include "../Enums.h"
#include "../Margin.h"
#include "../Rect.h"

namespace MoonGlare::GUI::Component {

using namespace Core::Component;

union RectTransformComponentEntryFlagsMap {
    struct MapBits_t {
        bool m_Valid : 1;
        bool m_Dirty : 1;
        bool m_Changed : 1;
    };
    MapBits_t m_Map;
    uint8_t m_UintValue;

    void SetAll() {
        m_UintValue = 0;
        m_UintValue = ~m_UintValue;
    }
    void ClearAll() { m_UintValue = 0; }

    static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
};

struct RectTransformComponentEntry {
    RectTransformComponentEntryFlagsMap m_Flags;

    AlignMode m_AlignMode;
    uint8_t m_padding;
    uint16_t m_Z;

    Point m_Position; //not pod
    Point m_Size;     //not pod
    //TODO: margin property

    Margin m_Margin; //not pod

    math::mat4 m_GlobalMatrix; //not pod
    math::mat4 m_LocalMatrix;  //not pod
    Rect m_ScreenRect;         //not pod

    MoonGlare::Configuration::RuntimeRevision m_Revision;

    void Recalculate(RectTransformComponentEntry &Parent);

    void SetDirty() {
        m_Revision = 0;
        m_Flags.m_Map.m_Dirty = true;
    }

    void Reset() {
        m_Revision = 0;
        m_Flags.ClearAll();
    }
};
//static_assert((sizeof(RectTransformComponentEntry) % 16) == 0, "RectTransformComponentEntry has invalid size");
//static_assert(std::is_pod<RectTransformComponentEntry>::value, "RectTransformComponentEntry must be pod!");

struct RectTransformSettingsFlagsMap {
    struct MapBits_t {
        bool m_UniformMode : 1;
    };
    MapBits_t m_Map;
    uint8_t m_UintValue;

    void SetAll() {
        m_UintValue = 0;
        m_UintValue = ~m_UintValue;
    }
    void ClearAll() { m_UintValue = 0; }

    static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
};

using Core::Scripts::Component::ScriptComponent;

class RectTransformComponent : public AbstractSubsystem, public SubSystemIdWrap<SubSystemId::RectTransform> {
public:
    static constexpr char *Name = "RectTransform";
    static constexpr bool PublishID = true;

    void HandleEvent(const MoonGlare::Component::EntityDestructedEvent &event);

    bool Initialize() override;

    int PushToLua(lua_State *lua, Entity Owner) override;

    //old

    RectTransformComponent(SubsystemManager *Owner);
    virtual ~RectTransformComponent();
    virtual void Step(const Core::MoveConfig &conf) override;
    virtual bool Load(ComponentReader &reader, Entity parent, Entity owner) override;
    virtual bool LoadComponentConfiguration(pugi::xml_node node) override;

    ComponentIndex GetComponentIndex(Entity e) const { return values.entityMapper.GetIndex(e); }
    RectTransformComponentEntry *GetEntry(Entity e) {
        auto index = values.entityMapper.GetIndex(e);
        if (index == ComponentIndex::Invalid)
            return nullptr;
        return &values.entry[index];
    }

    RectTransformComponentEntry &GetRootEntry() { return values.entry[0]; }
    const RectTransformComponentEntry &GetRootEntry() const { return values.entry[0]; }

    bool IsUniformMode() const { return m_Flags.m_Map.m_UniformMode; }
    const Renderer::VirtualCamera &GetCamera() const { return m_Camera; }

    static MoonGlare::Scripts::ApiInitializer RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root);
    static void RegisterDebugScriptApi(ApiInitializer &root);

    math::vec2 PixelToCurrent(math::vec2 pix) const {
        if (!IsUniformMode())
            return pix;
        return pix / m_ScreenSize * GetRootEntry().m_Size;
    }

protected:
    ScriptComponent *m_ScriptComponent;
    Component::EntityManager *entityManager;
    RectTransformSettingsFlagsMap m_Flags;
    MoonGlare::Configuration::RuntimeRevision m_CurrentRevision;
    Point m_ScreenSize;
    Renderer::VirtualCamera m_Camera;

    class RectTransformDebugDraw;
    std::unique_ptr<RectTransformDebugDraw> debugDraw;

    struct LuaWrapper;
    struct Values : public MoonGlare::Memory::PackedArrayIndexTree<
                        ComponentIndex, (ComponentIndex)MoonGlare::Configuration::Storage::ComponentBuffer, Values> {
        Array<Entity> owner;
        Array<RectTransformComponentEntry> entry;

        EntityArrayMapper<> entityMapper;

        RectTransformComponent *component;

        void ClearArrays() { entityMapper.Fill(ElementIndex::Invalid); }
        void SwapValues(ElementIndex a, ElementIndex b) {
            entityMapper.Swap(owner[a], owner[b]);
            std::swap(owner[a], owner[b]);
            std::swap(entry[a], entry[b]);
        }
        void ReleaseElement(ElementIndex e, ElementIndex parent) {
            entityMapper.SetIndex(owner[e], InvalidIndex);
            component->ElementRemoved(e);
            //owner[e] = Entity::Invalid;
            owner[e] = {};
        }
        void InitElement(ElementIndex e, ElementIndex parent) {}
    };
    //static_assert(std::is_trivial_v<Values>);
    Values values;

    void ElementRemoved(Values::ElementIndex index);
};

} //namespace MoonGlare::GUI::Component
