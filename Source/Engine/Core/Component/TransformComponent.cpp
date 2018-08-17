#include <pch.h>
#include <MoonGlare.h>

#include "TransformComponent.h"
#include "SubsystemManager.h"
#include "ComponentRegister.h"

#include <Foundation/Component/EventDispatcher.h>
#include <Foundation/Component/EntityManager.h>

#include <Math.x2c.h>
#include <ComponentCommon.x2c.h>
#include <TransformComponent.x2c.h>

namespace MoonGlare::Core::Component {

RegisterComponentID<TransformComponent> TransformComponentIDReg("Transform");

TransformComponent::TransformComponent(SubsystemManager * Owner)
		: iSubsystem(), subSystemManager(Owner) {
    
    entityManager = Owner->GetInterfaceMap().GetInterface<Component::EntityManager>();
    assert(entityManager);
           
    values.component = this;

    m_CurrentRevision = 0;
}

TransformComponent::~TransformComponent() {}

bool TransformComponent::Initialize() {
    auto &ed = GetManager()->GetEventDispatcher();
    ed.Register<Component::EntityDestructedEvent>(this);

    scriptComponent = GetManager()->GetComponent<Scripts::Component::ScriptComponent>();
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

struct TransformComponent::LuaWrapper {
    TransformComponent *component;
    Scripts::Component::ScriptComponent *scriptComponent;
    Entity owner;
    mutable ComponentIndex index;

    void Check() const {
        //if (transformComponent->componentIndexRevision != indexRevision) {
            index = component->GetComponentIndex(owner);
        //}
        if (index == ComponentIndex::Invalid) {
            __debugbreak();
            throw Scripts::LuaPanic("Attempt to dereference deleted Transform component! ");
        }
    }

    math::vec3 GetPosition() const { Check(); return emath::MathCast<math::fvec3>(component->GetPosition(index)); }
    void SetPosition(math::vec3 pos) { Check(); component->SetPosition(index, emath::MathCast<emath::fvec3>(pos)); }
    math::vec4 GetRotation() const {  Check(); return emath::MathCast<math::fvec4>(component->GetRotation(index)); }
    void SetRotation(math::vec4 rot) { Check(); component->SetRotation(index, emath::MathCast<emath::Quaternion>(rot)); }
    math::vec3 GetScale() const { Check(); return emath::MathCast<math::fvec3>(component->GetScale(index)); }
    void SetScale(math::vec3 s) { Check(); component->SetScale(index, emath::MathCast<emath::fvec3>(s)); }

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

MoonGlare::Scripts::ApiInitializer TransformComponent::RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root) {
	return root
	.beginClass<LuaWrapper>("TransformComponent")
        .addCFunction("GetParent", &LuaWrapper::GetParent)
        .addCFunction("GetFirstChild", &LuaWrapper::GetFirstChild)
        .addCFunction("GetNextSibling", &LuaWrapper::GetNextSibling)
        .addCFunction("FindChildByName", &LuaWrapper::FindChildByName)

		.addProperty("Position", &LuaWrapper::GetPosition, &LuaWrapper::SetPosition)
		.addProperty("Rotation", &LuaWrapper::GetRotation, &LuaWrapper::SetRotation)
		.addProperty("Scale", &LuaWrapper::GetScale, &LuaWrapper::SetScale)
        //TODO: find child
        //TODO: get parent
        //TODO: get all children ?
	.endClass()
	;
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

void TransformComponent::Step(const SubsystemUpdateData & conf) {
    ++m_CurrentRevision;
    //TODO: revision will overrun each 800 days of execution!!!!
    //if (m_CurrentRevision < 1) { m_CurrentRevision = 1; }

	for (size_t i = 1; i < values.Allocated(); ++i) {//ignore root entry
        ComponentIndex index = (ComponentIndex)i;

        auto parentIndex = values.parentIndex[index];
        assert(parentIndex != values.InvalidIndex);

        bool dirty = values.flags[index].dirty;
		if (dirty || (values.revision[parentIndex] > values.revision[index])) {

            emath::Transform tr;
            tr.setIdentity();
            tr.rotate(values.quaternion[index]);
            tr.translation() = values.position[index];
            tr.scale(values.scale[index]);

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

	auto index = values.Allocate(parentIndex);
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
    values.quaternion[index]    = emath::MathCast<emath::Quaternion>(te.m_Rotation);
    values.scale[index]         = emath::MathCast<emath::fvec3>(te.m_Scale);
    values.globalScale[index]   = values.scale[index];

    values.revision[index]      = 0;

	return true;
}

} 
