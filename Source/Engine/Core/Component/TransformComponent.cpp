
#include <pch.h>
#include <MoonGlare.h>

#include "TransformComponent.h"
#include "SubsystemManager.h"
#include "ComponentRegister.h"

#include <Foundation/Component/EventDispatcher.h>

#include <Math.x2c.h>
#include <ComponentCommon.x2c.h>
#include <TransformComponent.x2c.h>

namespace MoonGlare::Core::Component {

RegisterComponentID<TransformComponent> TransformComponentIDReg("Transform", true, &TransformComponent::RegisterScriptApi);

TransformComponent::TransformComponent(SubsystemManager * Owner)
		: iSubsystem(), subSystemManager(Owner) {
    
    values.Clear();
    values.component = this;

    m_CurrentRevision = 0;
}

TransformComponent::~TransformComponent() {}

bool TransformComponent::Initialize() {
    auto &ed = GetManager()->GetEventDispatcher();
    ed.Register<Component::EntityDestructedEvent>(this);

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
    TransformComponent *transformComponent;
    Entity owner;
    mutable ComponentIndex index;

    void Check() const {
        //if (transformComponent->componentIndexRevision != indexRevision) {
            index = transformComponent->GetComponentIndex(owner);
        //}
    }

    math::vec3 GetPosition() const { 
        Check();
        return emath::MathCast<math::fvec3>(transformComponent->GetPosition(index));
    }
    void SetPosition(math::vec3 pos) {
        Check();
        transformComponent->SetPosition(index, emath::MathCast<emath::fvec3>(pos));
    }
    math::vec4 GetRotation() const { 
        Check();
        return emath::MathCast<math::fvec4>(transformComponent->GetRotation(index));
    }
    void SetRotation(math::vec4 rot) {
        Check();
        transformComponent->SetRotation(index, emath::MathCast<emath::Quaternion>(rot));
    }
    math::vec3 GetScale() const { 
        Check();
        return emath::MathCast<math::fvec3>(transformComponent->GetScale(index));
    }
    void SetScale(math::vec3 s) {
        Check();
        transformComponent->SetScale(index, emath::MathCast<emath::fvec3>(s));
    }
};

void TransformComponent::RegisterScriptApi(ApiInitializer &root) {
	root
	.beginClass<LuaWrapper>("TransformComponent")
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
    LuaWrapper lw{ this, owner, index, };
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
    GetManager()->GetWorld()->GetEntityManager()->Release(values.owner[index]);
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
