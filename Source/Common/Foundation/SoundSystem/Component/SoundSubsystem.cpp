#include <InterfaceMap.h>
#include "SoundSubsystem.h"
#include "SoundSourceComponent.h"
#include "SoundStreamFinishedEvent.h"

#include "../iSoundSystem.h"

#include <Foundation/Component/ComponentArray.h>
#include <Foundation/Component/EventDispatcher.h>

#include <SoundSourceComponent.x2c.h>
                           
namespace MoonGlare::SoundSystem::Component {

HandleApi SoundSourceComponent::handleApi;

//---------------------------------------------------------

SoundSubsystem::SoundSubsystem(iSubsystemManager *subsystemManager)
    : iSubsystem(), subsystemManager(subsystemManager) {

    handleApi = subsystemManager->GetInterfaceMap().GetInterface<iSoundSystem>()->GetHandleApi();
    componentArray = &subsystemManager->GetComponentArray();

    SoundSourceComponent::handleApi = handleApi;

    subsystemManager->GetEventDispatcher().Register<ComponentCreatedEvent>(this);
}

SoundSubsystem::~SoundSubsystem() {
    //todo: remove this workaround
    componentArray->ForEach<SoundSourceComponent>([this](uint32_t index, SoundSourceComponent& ssc) {
        handleApi.SetCallback(ssc.handle, nullptr, 0);
        handleApi.Close(ssc.handle, false);
    });

    SoundSourceComponent::handleApi = HandleApi();
}

void SoundSubsystem::Update(const SubsystemUpdateData &data) {
    componentArray->ForEach<SoundSourceComponent>([this](uint32_t index, SoundSourceComponent& ssc) {
        if (ssc.finishEvent) {
            //TODO        
            subsystemManager->GetEventDispatcher().Send(SoundStreamFinishedEvent{ ssc.e,ssc.e });
            ssc.finishEvent = false;
        }

        if (ssc.autostart) {
            ssc.autostart = false;
            handleApi.Play(ssc.handle);
        }
    });
}

void SoundSubsystem::HandleEvent(const ComponentCreatedEvent &ev) {
    SoundSourceComponent &ssc = componentArray->GetComponent<SoundSourceComponent>(ev.sender.GetIndex());
    ssc.handle = handleApi.Open("", false);
    handleApi.SetCallback(ssc.handle, &playbackWatcher, ev.sender.GetIndex());
}

void SoundSubsystem::OnPlaybackFinished(SoundHandle handle, bool loop, UserData userData) {
    auto *ssc = componentArray->QuerryComponent<SoundSourceComponent>(userData);
    if (ssc)
        ssc->finishEvent = true;
}

bool SoundSubsystem::Load(pugi::xml_node node, Entity Owner, Handle &hout) {
    x2c::SoundSystem::SoundSourceComponentData_t entry;
    if(!entry.Read(node))
        return false;

    SoundSourceComponent &ssc = componentArray->AssignComponent<SoundSourceComponent>(Owner.GetIndex());

    ssc.e = Owner;
    ssc.finishEvent = false;
    ssc.autostart = entry.state == SoundState::Playing;
    ssc.handle = SoundHandle::Invalid;
    ssc.handle = handleApi.Open(entry.uri, false, entry.kind, false);
    handleApi.SetCallback(ssc.handle, &playbackWatcher, Owner.GetIndex());

    return true;
}

}
