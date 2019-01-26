#include <pch.h>

#include <nfMoonGlare.h>

#include <Foundation/Component/EntityEvents.h>
#include <Foundation/Component/EntityManager.h>
#include <Foundation/Component/EventDispatcher.h>

#include "TransformComponent.h"
#include "TransformComponentLuaWrapper.h"

#include "Core/Component/ComponentRegister.h"
#include "Core/Component/SubsystemManager.h"

#include <ComponentCommon.x2c.h>
#include <Math.x2c.h>
#include <TransformComponent.x2c.h>

namespace MoonGlare::Component {

RegisterComponentID<TransformComponent> TransformComponentIDReg("Transform");

TransformComponent::TransformComponent(Core::Component::SubsystemManager * Owner)
        : iSubsystem(), subSystemManager(Owner) {
    
    entityManager = Owner->GetInterfaceMap().GetInterface<Component::EntityManager>();
    assert(entityManager);
           
    values.component = this;

    m_CurrentRevision = 0;
}

TransformComponent::~TransformComponent() {}

bool TransformComponent::Initialize() {
    auto &ed = GetManager()->GetEventDispatcher();
    ed.Register<EntityDestructedEvent>(this);

    scriptComponent = GetManager()->GetComponent<Core::Scripts::Component::ScriptComponent>();
    if (!scriptComponent) {
        AddLog(Error, "Failed to get ScriptComponent instance!");
        return false;
    }

    values.Clear();
    ComponentIndex index = values.Allocate();

    values.flags[index].ClearAll();
    values.owner[index] = GetManager()->GetRootEntity();
    values.revision[index] = m_CurrentRevision;

    values.globalScale[index] = emath::fvec3::Ones();
    values.globalTransform[index] = emath::Transform::Identity();

    values.scale[index] = emath::fvec3::Ones();
    values.position[index] = emath::fvec3::Zero();
    values.quaternion[index] = emath::Quaternion::Identity();

    auto owner = GetManager()->GetRootEntity();
    values.owner[index] = owner;
    values.entityMapper.SetIndex(owner, index);

    return true;
}

//---------------------------------------------------------------------------------------

MoonGlare::Scripts::ApiInitializer TransformComponent::RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root) {
    return LuaWrapper::RegisterScriptApi(root);
}

int TransformComponent::PushToLua(lua_State *lua, Entity owner) {
    auto index = values.entityMapper.GetIndex(owner);
    if (index == values.InvalidIndex)
        return 0;
    LuaWrapper lw{ this, scriptComponent, owner, index, };
    luabridge::push<LuaWrapper>(lua, lw);
    return 1;
}

//------------------------------------------------------------------------------------------

void TransformComponent::HandleEvent(const MoonGlare::Component::EntityDestructedEvent &event) {
    auto index = values.entityMapper.GetIndex(event.entity);
    if (index == values.InvalidIndex)
        return;

    //begin destruction
    values.RemoveBranch(index);
}

//------------------------------------------------------------------------------------------

void TransformComponent::ElementRemoved(Values::ElementIndex index) {
    entityManager->Release(values.owner[index]);
}

//------------------------------------------------------------------------------------------

ComponentIndex TransformComponent::GetComponentIndex(Entity e) const { 
    auto ci = values.entityMapper.GetIndex(e);
    if (ci != ComponentIndex::Invalid && e != values.owner[ci])
        return ComponentIndex::Invalid;
    return ci;
}

void TransformComponent::Step(const SubsystemUpdateData & conf) {
    ++m_CurrentRevision;
    //TODO: revision will overrun each 800 days of execution!!!!
    //if (m_CurrentRevision < 1) { m_CurrentRevision = 1; }

    for (size_t i = 1; i < values.Allocated(); ++i) {//ignore root entry
        ComponentIndex index = (ComponentIndex)i;

        auto parentIndex = values.parentIndex[index];
        assert(parentIndex != values.InvalidIndex);
        assert(parentIndex < index);

        bool dirty = values.flags[index].dirty;
        if (dirty || (values.revision[parentIndex] > values.revision[index])) {

            auto &revision = values.revision[index];
            auto &parentRevision = values.revision[parentIndex];

            auto &position = values.position[index];
            auto &quaternion = values.quaternion[index];
            auto &scale = values.scale[index];

            auto &myGlob = values.globalTransform[index];
            auto &parentGlob = values.globalTransform[parentIndex];

            auto &globalScale = values.globalScale[parentIndex];
            auto &parentGlobalScale = values.globalScale[parentIndex];

            auto &owner = values.owner[index];
            auto &parent = values.owner[parentIndex];

            auto idx = owner.GetIndex();

            emath::Transform tr;
            //emath::Transform &tr = values.localTransform[index];
            tr.setIdentity();
            tr.rotate(values.quaternion[index]);
            tr.scale(values.scale[index]);
            tr.translation() = values.position[index];

            values.globalTransform[index] = values.globalTransform[parentIndex] * tr;
            values.globalScale[index] = values.scale[index].cwiseProduct(values.globalScale[parentIndex]);

            if (dirty)
                values.revision[index] = m_CurrentRevision;
            else
                values.revision[index] = values.revision[parentIndex];

            values.flags[index].dirty = false;
        }
    }
}

bool TransformComponent::Load(ComponentReader &reader, Entity parent, Entity owner) {
    auto parentIndex = values.entityMapper.GetIndex(parent);
    if (parentIndex == values.InvalidIndex) {
        return false;
    }

    auto expectedParent = values.owner[parentIndex];


    auto idx = owner.GetIndex();
    auto gen = owner.GetGeneration();

    auto index = values.Allocate(parentIndex);

    assert(parentIndex < index);

    values.flags[index].ClearAll();
    values.flags[index].dirty = true;

    values.owner[index] = owner;
    values.entityMapper.SetIndex(owner, index);

    x2c::Component::TransformComponent::TransformEntry_t te;
    te.ResetToDefault();
    if (!reader.Read(te)) {
        AddLog(Error, "Failed to read TransfromEntry!");
        return false;
    }

    values.position[index]      = emath::MathCast<emath::fvec3>(te.m_Position);
    values.quaternion[index]    = emath::MathCast<emath::Quaternion>(te.m_Rotation).normalized();
    values.scale[index]         = emath::MathCast<emath::fvec3>(te.m_Scale);
    values.globalScale[index]   = values.scale[index];

    emath::Transform tr;
    tr.setIdentity();
    tr.rotate(values.quaternion[index]);
    tr.scale(values.scale[index]);
    tr.translation() = values.position[index];

    values.globalTransform[index] = values.globalTransform[parentIndex] * tr;
    values.globalScale[index] = values.scale[index].cwiseProduct(values.globalScale[parentIndex]);

    values.revision[index] = 0;

    return true;
}

//------------------------------------------------------------------------------------------

std::optional<Entity> TransformComponent::FindChildByName(Entity root, std::string_view name) {
    auto *em = entityManager;
    auto index = GetComponentIndex(root);
    for (auto childIndex : values.TraverseTree(index)) {
        Entity e = values.owner[childIndex];
        std::string n;
        if (em->GetEntityName(e, n) && name == n) {
            return e;
        }
    }
    return std::nullopt;
}

Entity TransformComponent::GetOwner(Entity item) {
    auto index = GetComponentIndex(item);
    auto parentIndex = values.parentIndex[index];
    if (parentIndex == values.InvalidIndex)
        return {};
    return values.owner[parentIndex];
}

} 
