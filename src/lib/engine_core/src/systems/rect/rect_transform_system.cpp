#pragma once

#include "rect_transform_system.hpp"
#include "component/rect/rect_transform.hpp"
#include "ecs/component_array.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::Systems::Rect {

using namespace Component::Rect;

namespace {
void RecalculateRectTransform(const RectTransform &parent, RectTransform &child) {
    const auto &parentmargin = parent.margin;
    const auto parentsize = GetRectSize(parent.screen_rect);

    bool doslice = true;

    auto update_position = [&](auto v) {
        child.position = math::fvec3(v[0], v[1], child.position[2]);
    };

    switch (child.align_mode) {
    case AlignMode::None:
        break;

    case AlignMode::Top:
        update_position(LeftTop(parentmargin));
        child.size[0] = parentsize[0] - Vertical(parentmargin);
        break;
    case AlignMode::Bottom:
        child.size[0] = parentsize[0] - Vertical(parentmargin);
        // update_position(Point(parentmargin.Left, parentsize[1] - parentmargin.Top - child.size[1]));
        break;
    case AlignMode::Left:
        update_position(LeftTop(parentmargin));
        child.size[1] = parentsize[1] - Vertical(parentmargin);
        break;
    case AlignMode::Right:
        // update_position(Point(parentsize[0] - parentmargin.Right - child.size[0], parentmargin.Top));
        child.size[1] = parentsize[1] - Vertical(parentmargin);
        break;

    case AlignMode::LeftTop:
        update_position(LeftTop(parentmargin));
        break;
    case AlignMode::LeftBottom:
        // update_position(Point(parentmargin.Left, parentsize[1] - parentmargin.Bottom - child.size[1]));
        break;
    case AlignMode::RightTop:
        // update_position(Point(parentsize[0] - parentmargin.Right - child.size[0], parentmargin.Top));
        break;
    case AlignMode::RightBottom:
        // update_position(parentsize - parentmargin.RightBottomMargin() - child.size);
        break;

    case AlignMode::LeftMiddle:
        // child.position =
        //     Point(parentmargin.Left,
        //           parentmargin.Top + (parentsize[1] - Vertical(parentmargin)) / 2.0f);
        break;
    case AlignMode::RightMiddle:
        // update_position(Point(parentsize[0] - parentmargin.Right - child.size[0]),
        //                    parentmargin.Top +
        //                        (parentsize[1] - Vertical(parentmargin) - child.size[1]) / 2.0f);
        break;
    case AlignMode::MiddleTop:
        // update_position(Point(parentmargin.Left )+
        //                        (parentsize[0] - parentmargin.HorizontalMargin() - child.size[0]) / 2.0f,
        //                    parentmargin.Top);
        break;
    case AlignMode::MiddleBottom:
        // update_position(Point(parentmargin.Left )+
        //                        (parentsize[0] - parentmargin.HorizontalMargin() - child.size[0]) / 2.0f,
        //                    parentsize[1] - parentmargin.Top - child.size[1]);
        break;

    case AlignMode::FillParent:
        update_position(LeftTop(parentmargin));
        child.size =
            Point(parentsize[0] - Horizontal(parentmargin), parentsize[1] - Vertical(parentmargin));
        break;

    case AlignMode::Center: {
        auto halfparent = parentsize / 2.0f;
        auto halfsize = child.size / 2.0f;
        update_position(halfparent - halfsize);
        break;
    }
    case AlignMode::Table: {
        auto cell = math::fvec2(parentsize - TotalMargin(parentmargin));
        cell[0] = (cell[0] / child.size[0]) * child.position[0];
        cell[1] = (cell[1] / child.size[1]) * child.position[1];
        auto cellpos = math::fvec2(LeftTop(parentmargin) + cell);
        // m_ScreenRect.SliceFromparent(parent.m_ScreenRect, cellpos, cell);
        // m_LocalMatrix = glm::translate(glm::identity<glm::fmat4>(), math::vec3(cellpos, 0));
        doslice = false;
        break;
    }

        // default:
        // LogInvalidEnum(m_AlignMode);
        // return;
    }

    if (doslice) {
        // m_ScreenRect.SliceFromparent(parent.m_ScreenRect, child.position, child.size);
        // m_LocalMatrix = glm::translate(glm::identity<glm::fmat4>(), math::vec3(child.position, 0));
    }

    // m_GlobalMatrix = parent.m_GlobalMatrix * m_LocalMatrix;
}

} // namespace

void RectTransformSystem::DoStep(double time_delta) {
    ++current_revision;

    GetComponentArray()->VisitWithParent<RectTransform>(
        [this](const RectTransform &parent, RectTransform &child) {
            if (parent.revision > child.revision) {
                RecalculateRectTransform(parent, child);
                child.revision = current_revision;

                // auto parentindex = values.parentindex[i];
                // assert(parentindex != values.InvalidIndex);

                // auto *parententry = &values.entry[parentindex];
                // if (parententry->m_Revision <= item.m_Revision && !item.m_Flags.map.m_Dirty) {
                //     //nothing to do, nothing changed;
                //     item.m_Flags.map.m_Changed = false;
                // } else {
                //     item.Recalculate(*parententry);
                //     item.m_Revision = m_CurrentRevision;
                //     item.m_Flags.map.m_Changed = true;
                //     //item.m_Flags.map.m_Dirty = false;
                // }
                // //	item.m_GlobalScale = parententry->m_GlobalScale * item.m_LocalScale;
            }
        });
}

} // namespace MoonGlare::Systems::Rect

#if 0

namespace MoonGlare::GUI::Component {

union RectTransformComponentEntryFlagsMap {
    struct MapBits_t {
        bool m_Valid : 1;
        bool m_Dirty : 1;
        bool m_Changed : 1;
    };
    MapBits_t map;
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

    Point child.position; //not pod
    Point child.size;     //not pod
    //TODO: margin property

    Margin margin; //not pod

    math::mat4 m_GlobalMatrix; //not pod
    math::mat4 m_LocalMatrix;  //not pod
    Rect m_ScreenRect;         //not pod

    MoonGlare::Configuration::RuntimeRevision m_Revision;

    void Recalculate(RectTransformComponentEntry &parent);

    void SetDirty() {
        m_Revision = 0;
        m_Flags.map.m_Dirty = true;
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
    MapBits_t map;
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

    bool IsUniformMode() const { return m_Flags.map.m_UniformMode; }
    const Renderer::VirtualCamera &GetCamera() const { return m_Camera; }

    static MoonGlare::Scripts::ApiInitializer RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root);
    static void RegisterDebugScriptApi(ApiInitializer &root);

    math::vec2 PixelToCurrent(math::vec2 pix) const {
        if (!IsUniformMode())
            return pix;
        return pix / m_ScreenSize * GetRootEntry().child.size;
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

namespace MoonGlare::GUI::Component {

#ifdef DEBUG
static bool gRectTransformDebugDraw = false;
#endif

//---------------------------------------------------------------------------------------

struct RectTransformComponent::LuaWrapper {
    RectTransformComponent *component;
    ScriptComponent *scriptComponent;

    Entity owner;
    mutable ComponentIndex index;

    void Check() const {
        //if (transformComponent->componentIndexRevision != indexRevision) {
        index = component->GetComponentIndex(owner);
        //}
        if (index == ComponentIndex::Invalid) {
            __debugbreak();
            throw Scripts::LuaPanic("Attempt to dereference deleted RectTransform component!");
        }
    }
    void SetDirty() { component->values.entry[index].m_Flags.map.m_Dirty = true; }

    math::fvec2 GetPosition() const { Check(); return component->values.entry[index].child.position; }
    void SetPosition(math::vec2 pos) { Check(); SetDirty(); component->values.entry[index].child.position = pos; }

    math::fvec2 GetSize() const { Check(); return component->values.entry[index].child.size; }
    void SetSize(math::vec2 pos) { Check(); SetDirty(); component->values.entry[index].child.size = pos; }

    math::fvec2 GetScreenPosition() const { Check(); return component->values.entry[index].m_ScreenRect.LeftTop; }
    void SetScreenPosition(math::vec2 pos) { Check(); SetDirty(); component->values.entry[index].m_ScreenRect.LeftTop = pos; }

    const char* GetAlignMode() const { Check(); return AlignModeEnum::ToString(component->values.entry[index].m_AlignMode).c_str(); }
    void SetAlignMode(const char *val) { Check(); SetDirty(); component->values.entry[index].m_AlignMode = AlignModeEnum::ConvertSafe(val); }

    math::vec2 PixelToCurrent(math::vec2 pos) const { return component->PixelToCurrent(pos); }

    int FindChildByName(lua_State *lua) {
        Check();
        std::string_view name = lua_tostring(lua, 2);
        if (name.empty())
            return 0;

        auto *em = component->entityManager;
        for (auto childIndex : component->values.TraverseTree(index)) {
            Entity e = component->values.owner[childIndex];
            std::string n;
            if (em->GetEntityName(e, n) && name == n) {
                return scriptComponent->GetGameObject(lua, e);
            }
        }
        return 0;
    }

    int FindChildAtPosition(lua_State *lua) {
        Check();
        auto pos = luabridge::Stack<math::vec2>::get(lua, 2);

        pos += component->GetRootEntry().m_ScreenRect.LeftTop;

        auto *parententry = &component->values.entry[component->values.parentindex[index]];
        if (!parententry->m_ScreenRect.IsPointInside(pos))
            return false;

        for (auto childIndex : component->values.TraverseTree(index)) {
            auto *entry = &component->values.entry[childIndex];
            if (entry->m_ScreenRect.IsPointInside(pos)) {
                Entity e = component->values.owner[childIndex];
                return scriptComponent->GetGameObject(lua, e);
            }
        }
        return 0;
    }

    int Getparent(lua_State *lua) {
        Check();
        auto parentindex = component->values.parentindex[index];
        if (parentindex == component->values.InvalidIndex)
            return 0;
        auto parententity = component->values.owner[parentindex];
        return scriptComponent->GetGameObject(lua, parententity);
    }
    int GetFirstChild(lua_State *lua) {
        Check();
        auto firstChild = component->values.firstChild[index];
        if (firstChild == component->values.InvalidIndex)
            return 0;
        auto childEntity = component->values.owner[firstChild];
        return scriptComponent->GetGameObject(lua, childEntity);
    }
    int GetNextSibling(lua_State *lua) {
        Check();
        auto nextSibling = component->values.nextSibling[index];
        if (nextSibling == component->values.InvalidIndex)
            return 0;
        auto siblingEntity = component->values.owner[nextSibling];
        return scriptComponent->GetGameObject(lua, siblingEntity);
    }
};

MoonGlare::Scripts::ApiInitializer RectTransformComponent::RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root) {
    return root
        .beginClass<LuaWrapper>("RectTransformComponent")
            .addCFunction("Getparent", &LuaWrapper::Getparent)
            .addCFunction("GetFirstChild", &LuaWrapper::GetFirstChild)
            .addCFunction("GetNextSibling", &LuaWrapper::GetNextSibling)
            .addCFunction("FindChildByName", &LuaWrapper::FindChildByName)

            .addCFunction("FindChildAtPosition", &LuaWrapper::FindChildAtPosition)
            .addFunction("PixelToCurrent", &LuaWrapper::PixelToCurrent)

            .addProperty("Position", &LuaWrapper::GetPosition, &LuaWrapper::SetPosition)
            .addProperty("Size", &LuaWrapper::GetSize, &LuaWrapper::SetSize)
            .addProperty("ScreenPosition", &LuaWrapper::GetScreenPosition, &LuaWrapper::SetScreenPosition)
            .addProperty("AlignMode", &LuaWrapper::GetAlignMode, &LuaWrapper::SetAlignMode)
        .endClass()
        ;
}

void RectTransformComponent::RegisterDebugScriptApi(ApiInitializer & root) {
    root
    .beginNamespace("Flags")
        .beginNamespace("RectTransformComponent")
#ifdef DEBUG
            .addVariable("DebugDraw", &gRectTransformDebugDraw)
#endif
        .endNamespace()
    .endNamespace();
    ;
}

int RectTransformComponent::PushToLua(lua_State *lua, Entity owner) {
    auto index = values.entityMapper.GetIndex(owner);
    if (index == values.InvalidIndex)
        return 0;
    LuaWrapper lw{ this, m_ScriptComponent, owner, index, };
    luabridge::push<LuaWrapper>(lua, lw);
    return 1;
}

//------------------------------------------------------------------------------------------

void RectTransformComponent::ElementRemoved(Values::ElementIndex index) {
    entityManager->Release(values.owner[index]);
}

//---------------------------------------------------------------------------------------

bool RectTransformComponent::Initialize() {
    values.Clear();
    values.component = this;
    m_CurrentRevision = 0;

    auto &ed = GetManager()->GetEventDispatcher();
    ed.Register<Component::EntityDestructedEvent>(this);

    m_ScriptComponent = GetManager()->GetComponent<ScriptComponent>();
    if (!m_ScriptComponent) {
        AddLog(Error, "Failed to get ScriptComponent instance!");
        return false;
    }

    size_t index = values.Allocate();

    auto &RootEntry = values.entry[index];
    RootEntry.m_Flags.ClearAll();
    RootEntry.m_Flags.map.m_Valid = true;
    values.owner[index] = GetManager()->GetRootEntity();

    m_ScreenSize = emath::MathCast<math::fvec2>(GetManager()->GetWorld()->GetRendererFacade()->GetContext()->GetSizef());

    if (m_Flags.map.m_UniformMode) {
        float Aspect = m_ScreenSize[0] / m_ScreenSize[1];
        RootEntry.m_ScreenRect.LeftTop = Point(-Aspect, -1.0f);
        RootEntry.m_ScreenRect.RightBottom = -RootEntry.m_ScreenRect.LeftTop;
    } else {
        RootEntry.m_ScreenRect.LeftTop = Point(0,0);
        RootEntry.m_ScreenRect.RightBottom = m_ScreenSize;
    }

    RootEntry.m_Revision = 1;
    RootEntry.child.position = RootEntry.m_ScreenRect.LeftTop;
    RootEntry.child.size = RootEntry.m_ScreenRect.GetSize();
    RootEntry.m_GlobalMatrix = glm::translate(glm::identity<glm::fmat4>(), math::vec3(RootEntry.m_ScreenRect.LeftTop, 1.0f));
    RootEntry.m_LocalMatrix = glm::identity<glm::fmat4>();

    auto &rb = RootEntry.m_ScreenRect;
    m_Camera.SetOrthogonalRect(rb.LeftTop[0], rb.LeftTop[1], rb.RightBottom[0], rb.RightBottom[1], -100.0f, 100.0f);

    values.entityMapper.SetIndex(values.owner[index], index);

    return true;
}

//---------------------------------------------------------------------------------------

void RectTransformComponent::HandleEvent(const MoonGlare::Component::EntityDestructedEvent &event) {
    auto index = values.entityMapper.GetIndex(event.entity);
    if (index == values.InvalidIndex)
        return;

    AddLog(Hint, "HandleEvent: index: " << index << " owner: " << values.owner[index]);

    //begin destruction
    values.RemoveBranch(index);
}

//---------------------------------------------------------------------------------------

bool RectTransformComponent::Load(ComponentReader &reader, Entity parent, Entity owner) {
    auto parentindex = values.entityMapper.GetIndex(parent);
    if (parentindex == values.InvalidIndex) {
        return false;
    }

    size_t index = values.Allocate(parentindex);
    values.entityMapper.SetIndex(owner, index);

    auto &entry = values.entry[index];
    entry.m_Flags.ClearAll();
    values.owner[index] = owner;

    x2c::Component::RectTransformComponent::RectTransformEntry_t rte;
    rte.ResetToDefault();
    if (!reader.Read(rte)) {
        AddLog(Error, "Failed to read RectTransfromEntry!");
        return false;
    }

    entry.m_AlignMode = rte.m_AlignMode;

    entry.m_ScreenRect.LeftTop = rte.child.position;
    entry.m_ScreenRect.RightBottom = rte.child.position + rte.child.size;

    entry.margin = rte.margin;
    entry.child.position = rte.child.position;
    entry.child.size = rte.child.size;

    int32_t rawz = static_cast<uint32_t>(rte.m_Z);
    rawz += static_cast<int32_t>(std::numeric_limits<uint16_t>::max()) / 2;
    entry.m_Z = static_cast<uint16_t>( rawz );

    if (rte.m_UniformMode != m_Flags.map.m_UniformMode) {
        auto &root = GetRootEntry();
        if (m_Flags.map.m_UniformMode) {
            //convert from pixel to uniform
            if (entry.m_AlignMode != AlignMode::Table) {
                auto half = root.child.size / 2.0f;
                entry.child.position = entry.child.position / m_ScreenSize;// -half;
                entry.child.size = entry.child.size / m_ScreenSize * root.child.size;
            }
            entry.margin = entry.margin / m_ScreenSize * root.child.size;
        } else {
            //convert from uniform to pixel
            //NOT TESTED; MAY NOT WORK
            float Aspect = m_ScreenSize[0] / m_ScreenSize[1];
            auto half = Point(Aspect, 1.0f);
            if (entry.m_AlignMode != AlignMode::Table) {
                entry.child.position = entry.child.position * m_ScreenSize + half;
                entry.child.size = entry.child.size * m_ScreenSize;
            }
            entry.margin *= m_ScreenSize;
        }
    }

    entry.Recalculate(values.entry[parentindex]);
    entry.m_Revision = m_CurrentRevision;

    values.entry[parentindex].m_Flags.map.m_Dirty = true;

    entry.m_Flags.map.m_Valid = true;
    entry.m_Flags.map.m_Dirty = true;

    return true;
}

bool RectTransformComponent::LoadComponentConfiguration(pugi::xml_node node) {
    x2c::Component::RectTransformComponent::RectTransformEntry_t rts;
    rts.ResetToDefault();
    if (!rts.Read(node)) {
        AddLog(Error, "Failed to read settings!");
        return false;
    }

    m_Flags.map.m_UniformMode = rts.m_UniformMode;

    return true;
}

//---------------------------------------------------------------------------------------


} //namespace MoonGlare::GUI::Component

#endif