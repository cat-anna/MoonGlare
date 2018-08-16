#include <InterfaceMap.h>

#include <Foundation/Component/EventDispatcher.h>

#include "SoundSourceComponent.h"
#include "SoundStreamFinishedEvent.h"

#include "../iSoundSystem.h"

#include <SoundSourceComponent.x2c.h>
                           
#include "SoundSourceComponentLuaWrap.h"

namespace MoonGlare::SoundSystem::Component {

SoundSourceComponent::SoundSourceComponent(iSubsystemManager *subsystemManager)
    : iSubsystem(), subsystemManager(subsystemManager) {
}

SoundSourceComponent::~SoundSourceComponent() {
}

bool SoundSourceComponent::Initialize() {
    values.Clear();
    values.component = this;
    values.handleApi = subsystemManager->GetInterfaceMap().GetInterface<iSoundSystem>()->GetHandleApi();
    
    subsystemManager->GetEventDispatcher().Register<EntityDestructedEvent>(this);

    return true;
}

bool SoundSourceComponent::Finalize() {
    values.RemoveAll();

    return true;
}

//---------------------------------------------------------

void SoundSourceComponent::Step(const SubsystemUpdateData &data) { }

//---------------------------------------------------------

int SoundSourceComponent::PushToLua(lua_State *lua, Entity owner) {
    auto index = values.entityMapper.GetIndex(owner);
    if (index == values.InvalidIndex)
        return 0;
    LuaWrapper lw{ this, owner, index, values.handleApi };
    luabridge::push<LuaWrapper>(lua, lw);
    return 1;
}

Scripts::ApiInitializer SoundSourceComponent::RegisterScriptApi(Scripts::ApiInitializer api) {
    return LuaWrapper::RegisterScriptApi(std::move(api));
}

//---------------------------------------------------------

void SoundSourceComponent::HandleEvent(const EntityDestructedEvent &event) {
    auto index = values.entityMapper.GetIndex(event.entity);
    if (index == values.InvalidIndex)
        return;
    values.ReleaseElement(index);
}

void SoundSourceComponent::OnPlaybackFinished(SoundHandle handle, bool loop, UserData userData) {
    Entity e = Entity::FromIntValue(userData);
    subsystemManager->GetEventDispatcher().Queue(SoundStreamFinishedEvent{ e, e });
}

//---------------------------------------------------------

bool SoundSourceComponent::Load(ComponentReader &reader, Entity parent, Entity owner) {
    x2c::SoundSystem::SoundSourceComponentData_t entry;
    entry.ResetToDefault();
    if(!reader.Read(entry))
        return false;

    auto index = values.Allocate();

    values.owner[index] = owner;
    values.soundHandle[index] = values.handleApi.Open(entry.uri, false, entry.kind, false);
    values.handleApi.SetCallback(values.soundHandle[index], &playbackWatcher, owner.GetIntValue());
    values.entityMapper.SetIndex(owner, index);

    return true;
}

bool SoundSourceComponent::Create(Entity owner) {
    bool exists = values.entityMapper.GetIndex(owner) != ComponentIndex::Invalid;
    if (exists)
        return true;

    auto index = values.Allocate();
    values.owner[index] = owner;
    values.soundHandle[index] = values.handleApi.Open("", false);
    values.handleApi.SetCallback(values.soundHandle[index], &playbackWatcher, owner.GetIntValue());
    values.entityMapper.SetIndex(owner, index);
    return true;
}

}
