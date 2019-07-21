#pragma once

#include <Memory/ArrayIndexTree.h>

#include <Foundation/Component/EntityEvents.h>
#include <Foundation/Component/iSubsystem.h>

#include <Core/Scripts/ScriptComponent.h>

#include "../Configuration.h"

namespace MoonGlare::Component {     
using namespace MoonGlare::Core::Component;

struct EntityDestructedEvent;

class TransformComponent 
    : public iSubsystem
    , public SubSystemIdWrap<SubSystemId::Transform>
{
public:
    TransformComponent(Core::Component::SubsystemManager *Owner);
    virtual ~TransformComponent();

    using RuntimeRevision = MoonGlare::Configuration::RuntimeRevision;

    bool Initialize() override;
    bool Load(Component::ComponentReader &reader, Entity parent, Entity owner) override;
    void Step(const SubsystemUpdateData &conf) override;
    int PushToLua(lua_State *lua, Entity Owner) override;

    void HandleEvent(const EntityDestructedEvent &event);

//------------------------------------------------------------------------------------------

    ComponentIndex GetComponentIndex(Entity e) const;
    
    void SetDirty(ComponentIndex ci) { values.flags[ci].dirty = true; }

    void SetScale(ComponentIndex ci, const emath::fvec3 &scale) { values.scale[ci] = scale; SetDirty(ci); }
    const emath::fvec3 &GetScale(ComponentIndex ci) const { return values.scale[ci]; }
    const emath::fvec3 &GetGlobalScale(ComponentIndex ci) const { return values.globalScale[ci]; }

    void SetPosition(ComponentIndex ci, const emath::fvec3 &pos) { values.position[ci] = pos; SetDirty(ci); }
    const emath::fvec3 &GetPosition(ComponentIndex ci) const { return values.position[ci]; }

    void SetRotation(ComponentIndex ci, const emath::Quaternion& quat) { values.quaternion[ci] = quat; SetDirty(ci); }
    const emath::Quaternion &GetRotation(ComponentIndex ci) const { return values.quaternion[ci]; }

    const emath::Transform &GetTransform(ComponentIndex ci) const { return values.globalTransform[ci]; }
    //const emath::Transform &GetLocalTransform(ComponentIndex ci) const { return values.localTransform[ci]; }
    //void SetLocalTransform(ComponentIndex ci, const emath::Transform & tr) { values.localTransform[ci] = tr; }
    RuntimeRevision GetRevision(ComponentIndex ci) const { return values.revision[ci]; }
    Entity GetIndexOwner(ComponentIndex ci) const { return values.owner[ci]; }

    std::optional<Entity> FindChildByName(Entity root, std::string_view name);
    Entity GetOwner(Entity item);

    RuntimeRevision GetCurrentRevision() const { return m_CurrentRevision; }
    static MoonGlare::Scripts::ApiInitializer RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root);
protected:
    Core::Component::SubsystemManager *subSystemManager;
    Core::Scripts::Component::ScriptComponent *scriptComponent;
    Component::EntityManager *entityManager;
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
    
    struct Values : public MoonGlare::Memory::ArrayIndexTree<ComponentIndex, (ComponentIndex)(1<<14), Values>{
        Array<Entity>               owner;
        Array<EntryFlags>           flags;
        Array<RuntimeRevision>      revision;
        Array<emath::fvec3>         scale;
        Array<emath::fvec3>         globalScale;
        Array<emath::fvec3>         position;
        Array<emath::Quaternion>    quaternion;
        //Array<emath::Transform>     localTransform;
        Array<emath::Transform>     globalTransform;

        EntityArrayMapper<>  entityMapper;

        TransformComponent      *component;

        void ClearArrays() {
            owner.fill({});
            flags.fill({});
            revision.fill(0);
            scale.fill({});
            globalScale.fill({});
            position.fill({});
            quaternion.fill({});
            //localTransform.fill({});
            globalTransform.fill({});
            entityMapper.Clear();
        }
        void SwapValues(ElementIndex a, ElementIndex b) {
            entityMapper.Swap(owner[a], owner[b]);
            std::swap(owner[a], owner[b]);
            std::swap(flags[a], flags[b]);
            std::swap(revision[a], revision[b]);
            std::swap(scale[a], scale[b]);
            std::swap(globalScale[a], globalScale[b]);
            std::swap(position[a], position[b]);
            std::swap(quaternion[a], quaternion[b]);
            std::swap(globalTransform[a], globalTransform[b]);
            //std::swap(localTransform[a], localTransform[b]);
        }
        void ReleaseElement(ElementIndex e, ElementIndex parent) {
            entityMapper.SetIndex(owner[e], InvalidIndex);
            component->ElementRemoved(e);
            //owner[e] = Entity::Invalid;
            owner[e] = {};
            revision[e] = 0;
            flags[e].dirty = true;
        }
        void InitElemenent(ElementIndex e, ElementIndex parent){
        }
    };
    //static_assert(std::is_trivial_v<Values>);
    Values values;

    void ElementRemoved(Values::ElementIndex index);
    Core::Component::SubsystemManager* GetManager() { return subSystemManager; }
};

}
