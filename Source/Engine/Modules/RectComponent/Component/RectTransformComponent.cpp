/*
  * Generated by cppsrc.sh
  * On 2016-09-12 17:58:33,31
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <nfMoonGlare.h>
#include "../nfGUI.h"

#include <Core/Component/SubsystemManager.h>
#include <Core/Component/ComponentRegister.h>
#include "RectTransformComponent.h"

#include <Math.x2c.h>
#include <ComponentCommon.x2c.h>
#include <RectTransformComponent.x2c.h>

#include <Source/Renderer/Renderer.h>
#include <Core/Scripts/LuaApi.h>

#include <Foundation/Component/EntityManager.h>

#include "../RectTransformDebugDraw.h"

namespace MoonGlare::GUI::Component {

::Space::RTTI::TypeInfoInitializer<RectTransformComponent, RectTransformComponentEntry> RectTransformComponentTypeInfo;
RegisterComponentID<RectTransformComponent> RectTransformComponentIDReg("RectTransform");
RegisterDebugApi(RectTransformComponent, &RectTransformComponent::RegisterDebugScriptApi, "Debug");

#ifdef DEBUG
static bool gRectTransformDebugDraw = false;
#endif

//---------------------------------------------------------------------------------------

RectTransformComponent::RectTransformComponent(SubsystemManager *Owner)
    : AbstractSubsystem(Owner) {
    entityManager = Owner->GetInterfaceMap().GetInterface<Component::EntityManager>();
    assert(entityManager);
}

RectTransformComponent::~RectTransformComponent() {
}

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
    void SetDirty() { component->values.entry[index].m_Flags.m_Map.m_Dirty = true; }

    math::fvec2 GetPosition() const { Check(); return component->values.entry[index].m_Position; }
    void SetPosition(math::vec2 pos) { Check(); SetDirty(); component->values.entry[index].m_Position = pos; }

    math::fvec2 GetSize() const { Check(); return component->values.entry[index].m_Size; }
    void SetSize(math::vec2 pos) { Check(); SetDirty(); component->values.entry[index].m_Size = pos; }

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

        auto *ParentEntry = &component->values.entry[component->values.parentIndex[index]];
        if (!ParentEntry->m_ScreenRect.IsPointInside(pos))
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

    int GetParent(lua_State *lua) {
        Check();
        auto parentIndex = component->values.parentIndex[index];
        if (parentIndex == component->values.InvalidIndex) 
            return 0;
        auto parentEntity = component->values.owner[parentIndex];
        return scriptComponent->GetGameObject(lua, parentEntity);
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
            .addCFunction("GetParent", &LuaWrapper::GetParent)
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
    RootEntry.m_Flags.m_Map.m_Valid = true;
    values.owner[index] = GetManager()->GetRootEntity();

    m_ScreenSize = emath::MathCast<math::fvec2>(GetManager()->GetWorld()->GetRendererFacade()->GetContext()->GetSizef());

    if (m_Flags.m_Map.m_UniformMode) {
        float Aspect = m_ScreenSize[0] / m_ScreenSize[1];
        RootEntry.m_ScreenRect.LeftTop = Point(-Aspect, -1.0f);
        RootEntry.m_ScreenRect.RightBottom = -RootEntry.m_ScreenRect.LeftTop;
    } else {
        RootEntry.m_ScreenRect.LeftTop = Point(0,0);
        RootEntry.m_ScreenRect.RightBottom = m_ScreenSize;
    }

    RootEntry.m_Revision = 1;
    RootEntry.m_Position = RootEntry.m_ScreenRect.LeftTop;
    RootEntry.m_Size = RootEntry.m_ScreenRect.GetSize();
    RootEntry.m_GlobalMatrix = glm::translate(glm::identity<glm::fmat4>(), math::vec3(RootEntry.m_ScreenRect.LeftTop, 1.0f));
    RootEntry.m_LocalMatrix = glm::identity<glm::fmat4>();

    auto &rb = RootEntry.m_ScreenRect;
    m_Camera.SetOrthogonalRect(rb.LeftTop.x, rb.LeftTop.y, rb.RightBottom.x, rb.RightBottom.y, -100.0f, 100.0f);

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

void RectTransformComponent::Step(const Core::MoveConfig & conf) {
    ++m_CurrentRevision;
    //TODO: revision will overrun each 800 days of execution!!!!
    //if (m_CurrentRevision < 1) { m_CurrentRevision = 1; }

    for (size_t i = 1; i < values.Allocated(); ++i) {//ignore root entry
        auto &item = values.entry[i];

        if (!item.m_Flags.m_Map.m_Valid) {
            //mark and continue
            continue;
        }

        auto parentIndex = values.parentIndex[i];
        assert(parentIndex != values.InvalidIndex);

        auto *ParentEntry = &values.entry[parentIndex];
        if (ParentEntry->m_Revision <= item.m_Revision && !item.m_Flags.m_Map.m_Dirty) {
            //nothing to do, nothing changed;
            item.m_Flags.m_Map.m_Changed = false;
        } else {
            item.Recalculate(*ParentEntry);
            item.m_Revision = m_CurrentRevision;
            item.m_Flags.m_Map.m_Changed = true;
            //item.m_Flags.m_Map.m_Dirty = false;
        }
        //	item.m_GlobalScale = ParentEntry->m_GlobalScale * item.m_LocalScale;
    }

#ifdef DEBUG
    if (gRectTransformDebugDraw) {
        if (!debugDraw)
            debugDraw = std::make_unique<RectTransformDebugDraw>();
        debugDraw->DebugDraw(conf, this);
    }
#endif
}

//---------------------------------------------------------------------------------------

bool RectTransformComponent::Load(ComponentReader &reader, Entity parent, Entity owner) {
    auto parentIndex = values.entityMapper.GetIndex(parent);
    if (parentIndex == values.InvalidIndex) {
        return false;
    }

    size_t index = values.Allocate(parentIndex);
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

    entry.m_ScreenRect.LeftTop = rte.m_Position;
    entry.m_ScreenRect.RightBottom = rte.m_Position + rte.m_Size;

    entry.m_Margin = rte.m_Margin;
    entry.m_Position = rte.m_Position;
    entry.m_Size = rte.m_Size;

    int32_t rawz = static_cast<uint32_t>(rte.m_Z);
    rawz += static_cast<int32_t>(std::numeric_limits<uint16_t>::max()) / 2;
    entry.m_Z = static_cast<uint16_t>( rawz );

    if (rte.m_UniformMode != m_Flags.m_Map.m_UniformMode) {
        auto &root = GetRootEntry();
        if (m_Flags.m_Map.m_UniformMode) {
            //convert from pixel to uniform
            if (entry.m_AlignMode != AlignMode::Table) {
                auto half = root.m_Size / 2.0f;
                entry.m_Position = entry.m_Position / m_ScreenSize;// -half;
                entry.m_Size = entry.m_Size / m_ScreenSize * root.m_Size;
            }
            entry.m_Margin = entry.m_Margin / m_ScreenSize * root.m_Size;
        } else {
            //convert from uniform to pixel
            //NOT TESTED; MAY NOT WORK
            float Aspect = m_ScreenSize[0] / m_ScreenSize[1];
            auto half = Point(Aspect, 1.0f);
            if (entry.m_AlignMode != AlignMode::Table) {
                entry.m_Position = entry.m_Position * m_ScreenSize + half;
                entry.m_Size = entry.m_Size * m_ScreenSize;
            }
            entry.m_Margin *= m_ScreenSize;
        }
    } 

    entry.Recalculate(values.entry[parentIndex]);
    entry.m_Revision = m_CurrentRevision;

    values.entry[parentIndex].m_Flags.m_Map.m_Dirty = true;

    entry.m_Flags.m_Map.m_Valid = true;
    entry.m_Flags.m_Map.m_Dirty = true;
    
    return true;
}

bool RectTransformComponent::LoadComponentConfiguration(pugi::xml_node node) {
    x2c::Component::RectTransformComponent::RectTransformEntry_t rts;
    rts.ResetToDefault();
    if (!rts.Read(node)) {
        AddLog(Error, "Failed to read settings!");
        return false;
    }

    m_Flags.m_Map.m_UniformMode = rts.m_UniformMode;
    
    return true;
}

//---------------------------------------------------------------------------------------

void RectTransformComponentEntry::Recalculate(RectTransformComponentEntry &Parent) {
    const auto &parentmargin = Parent.m_Margin;
    const auto parentsize = Parent.m_ScreenRect.GetSize();

    bool doslice = true;

    switch (m_AlignMode) {
    case AlignMode::None:
        break;

    case AlignMode::Top:
        m_Position = parentmargin.LeftTopMargin();
        m_Size.x = parentsize.x - parentmargin.VerticalMargin();
        break;
    case AlignMode::Bottom:
        m_Size.x = parentsize.x - parentmargin.VerticalMargin();
        m_Position = Point(parentmargin.Left, parentsize.y - parentmargin.Top - m_Size.y);
        break;
    case AlignMode::Left:
        m_Position = parentmargin.LeftTopMargin();
        m_Size.y = parentsize.y - parentmargin.VerticalMargin();
        break;
    case AlignMode::Right:
        m_Position = Point(parentsize.x - parentmargin.Right - m_Size.x, parentmargin.Top);
        m_Size.y = parentsize.y - parentmargin.VerticalMargin();
        break;

    case AlignMode::LeftTop:
        m_Position = parentmargin.LeftTopMargin();
        break;
    case AlignMode::LeftBottom:
        m_Position = Point(parentmargin.Left, parentsize.y - parentmargin.Bottom - m_Size.y);
        break;
    case AlignMode::RightTop:
        m_Position = Point(parentsize.x - parentmargin.Right - m_Size.x, parentmargin.Top);
        break;
    case AlignMode::RightBottom:
        m_Position = parentsize - parentmargin.RightBottomMargin() - m_Size;
        break;

    case AlignMode::LeftMiddle: 
        m_Position = Point(parentmargin.Left, parentmargin.Top + (parentsize.y - parentmargin.VerticalMargin()) / 2.0f);
        break;
    case AlignMode::RightMiddle: 
        m_Position = Point(parentsize.x - parentmargin.Right - m_Size.x, parentmargin.Top + (parentsize.y - parentmargin.VerticalMargin() - m_Size.y) / 2.0f);
        break;
    case AlignMode::MiddleTop: 
        m_Position = Point(parentmargin.Left + (parentsize.x - parentmargin.HorizontalMargin() - m_Size.x) / 2.0f, parentmargin.Top);
        break;
    case AlignMode::MiddleBottom: 
        m_Position = Point(parentmargin.Left + (parentsize.x - parentmargin.HorizontalMargin() - m_Size.x) / 2.0f, parentsize.y - parentmargin.Top - m_Size.y);
        break;

    case AlignMode::FillParent:
        m_Position = parentmargin.LeftTopMargin();
        m_Size = Point(parentsize.x - parentmargin.HorizontalMargin(), parentsize.y - parentmargin.VerticalMargin());
        break;

    case AlignMode::Center: {
        auto halfparent = parentsize / 2.0f;
        auto halfsize = m_Size / 2.0f;
        m_Position = halfparent - halfsize;
        break;
    }
    case AlignMode::Table: {
        auto cell = (parentsize - (parentmargin.TotalMargin())) / m_Size;
        auto cellpos = parentmargin.LeftTopMargin() + cell * m_Position;
        m_ScreenRect.SliceFromParent(Parent.m_ScreenRect, cellpos, cell);
        m_LocalMatrix = glm::translate(glm::identity<glm::fmat4>(), math::vec3(cellpos, 0));
        doslice = false;
        break;
    }

    default:
        LogInvalidEnum(m_AlignMode);
        return;
    }

    if (doslice) {
        m_ScreenRect.SliceFromParent(Parent.m_ScreenRect, m_Position, m_Size);
        m_LocalMatrix = glm::translate(glm::identity<glm::fmat4>(), math::vec3(m_Position, 0));
    }

    m_GlobalMatrix = Parent.m_GlobalMatrix * m_LocalMatrix;
}

} //namespace MoonGlare::GUI::Component
