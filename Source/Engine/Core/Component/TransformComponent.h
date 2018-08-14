#pragma once

#include <Foundation/Memory/ArrayIndexTree.h>

#include <Foundation/Component/iSubsystem.h>
#include <Foundation/Component/EntityEvents.h>

#include "../../Configuration.h"
#include "../EntityManager.h"
#include "nfComponent.h"

namespace MoonGlare::Core::Component {

using namespace MoonGlare::Component;

class TransformComponent 
	: public iSubsystem
	, public ComponentIDWrap<ComponentID::Transform>
{
public:
	TransformComponent(SubsystemManager *Owner);
	virtual ~TransformComponent();

    using RuntimeRevision = MoonGlare::Configuration::RuntimeRevision;

    bool Initialize() override;
	bool Load(Component::ComponentReader &reader, Entity parent, Entity owner) override;
	void Step(const SubsystemUpdateData &conf) override;
    int PushToLua(lua_State *lua, Entity Owner) override;

    void HandleEvent(const MoonGlare::Component::EntityDestructedEvent &event);

//------------------------------------------------------------------------------------------

    ComponentIndex GetComponentIndex(Entity e) const { return values.entityMapper.GetIndex(e); }
    
    void SetDirty(ComponentIndex ci) { values.flags[ci].dirty = true; }

    void SetScale(ComponentIndex ci, const emath::fvec3 &scale) { values.scale[ci] = scale; SetDirty(ci); }
    const emath::fvec3 &GetScale(ComponentIndex ci) const { return values.scale[ci]; }
    const emath::fvec3 &GetGlobalScale(ComponentIndex ci) const { return values.globalScale[ci]; }

    void SetPosition(ComponentIndex ci, const emath::fvec3 &pos) { values.position[ci] = pos; SetDirty(ci); }
    const emath::fvec3 &GetPosition(ComponentIndex ci) const { return values.position[ci]; }

    void SetRotation(ComponentIndex ci, const emath::Quaternion& quat) { values.quaternion[ci] = quat; SetDirty(ci); }
    const emath::Quaternion &GetRotation(ComponentIndex ci) const { return values.quaternion[ci]; }

    const emath::Transform &GetTransform(ComponentIndex ci) const { return values.globalTransform[ci]; }
    RuntimeRevision GetRevision(ComponentIndex ci) const { return values.revision[ci]; }

    RuntimeRevision GetCurrentRevision() const { return m_CurrentRevision; }
	static void RegisterScriptApi(ApiInitializer &root);
protected:
    SubsystemManager * subSystemManager;
    MoonGlare::Configuration::RuntimeRevision m_CurrentRevision = 0;

    struct LuaWrapper;

    union EntryFlags {
        struct {
            bool dirty : 1;
        };
        uint8_t UIntValue;

        void SetAll() { UIntValue = 0; UIntValue = ~UIntValue; }
        void ClearAll() { UIntValue = 0; }
    };
    static_assert(sizeof(EntryFlags) == sizeof(uint8_t));
	
    struct Values : public MoonGlare::Memory::ArrayIndexTree<ComponentIndex, (ComponentIndex)MoonGlare::Configuration::Storage::ComponentBuffer, Values>{
        Array<Entity>               owner;
        Array<EntryFlags>           flags;
        Array<RuntimeRevision>      revision;
        Array<emath::fvec3>         scale;
        Array<emath::fvec3>         globalScale;
        Array<emath::fvec3>         position;
        Array<emath::Quaternion>    quaternion;
        Array<emath::Transform>     globalTransform;

        EntityArrayMapper<ElementIndex>  entityMapper;

        TransformComponent      *component;

        void ClearArrays() {
            entityMapper.Fill(ElementIndex::Invalid);
        }
        void SwapValues(ElementIndex a, ElementIndex b) {
            std::swap(owner[a], owner[b]);
            std::swap(flags[a], flags[b]);
            std::swap(scale[a], scale[b]);
            std::swap(globalScale[a], globalScale[b]);
            std::swap(position[a], position[b]);
            std::swap(quaternion[a], quaternion[b]);
            std::swap(globalTransform[a], globalTransform[b]);

            entityMapper.SetIndex(owner[a], a);
            entityMapper.SetIndex(owner[b], b);
        }
        void ReleaseElement(ElementIndex e, ElementIndex parent) {
            entityMapper.SetIndex(owner[e], InvalidIndex);
            component->ElementRemoved(e);
            owner[e] = {};
        }
        void InitElemenent(ElementIndex e, ElementIndex parent){
        }

    };
    //static_assert(std::is_trivial_v<Values>);
    Values values;

    void ElementRemoved(Values::ElementIndex index);
    SubsystemManager* GetManager() { return subSystemManager; }
};

}
