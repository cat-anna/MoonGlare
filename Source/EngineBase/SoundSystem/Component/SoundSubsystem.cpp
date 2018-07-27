#include <InterfaceMap.h>
#include "SoundSubsystem.h"
#include "SoundSourceComponent.h"

#include "../iSoundSystem.h"

#include <EngineBase/Component/ComponentArray.h>
#include <EngineBase/Component/EventDispatcher.h>

#include "Events.h"

#include <SoundSourceComponent.x2c.h>
                           
namespace MoonGlare::SoundSystem::Component {

SoundSubsystem::SoundSubsystem(iSubsystemManager *subsystemManager)
    : iSubsystem(), subsystemManager(subsystemManager) {

    handleApi = subsystemManager->GetInterfaceMap().GetInterface<iSoundSystem>()->GetHandleApi();
    componentArray = &subsystemManager->GetComponentArray();
}

SoundSubsystem::~SoundSubsystem() {}

void SoundSubsystem::Update(const SubsystemUpdateData &data) {
    componentArray->ForEach<SoundSourceComponent>([this](uint32_t index, SoundSourceComponent& ssc) {

        if (ssc.looped) {
            //TODO        
            subsystemManager->GetEventDispatcher().SendTo(SoundStreamFinished{}, ssc.e);
            ssc.looped = false;
        }

        if (ssc.finished) {
            //TODO
            subsystemManager->GetEventDispatcher().SendTo(SoundStreamFinished{}, ssc.e);
            ssc.finished = false;
        }

        if (ssc.wantedState == SoundState::Invalid)
            return;

        auto currentState = handleApi.GetState(ssc.handle);
        if (currentState == SoundState::Invalid) {
            assert(false);
            handleApi.Close(ssc.handle, false);
            ssc.handle = SoundHandle::Invalid;
            componentArray->RemoveComponent<SoundSourceComponent>(index);
            return;
        }
        if (ssc.wantedState == currentState)
            return;

        switch (ssc.wantedState) {
        case SoundState::Playing:
            handleApi.Play(ssc.handle);
            break;
        case SoundState::Paused:
            handleApi.Pause(ssc.handle);
            break;
        case SoundState::Stopped:
            handleApi.Stop(ssc.handle);
            break;
        }
        ssc.wantedState = SoundState::Invalid;
    });
}

bool SoundSubsystem::Load(pugi::xml_node node, Entity Owner, Handle &hout) {
    x2c::SoundSystem::SoundSourceComponentData_t entry;
    if(!entry.Read(node))
        return false;

    SoundSourceComponent &ssc = componentArray->AssignComponent<SoundSourceComponent>(Owner.GetIndex());

    ssc.wantedState = entry.state;
    if (ssc.wantedState == SoundState::Stopped)
        ssc.wantedState = SoundState::Invalid;
    ssc.uri = entry.uri;
    ssc.handle = handleApi.Open(entry.uri, false, SoundKind::Music, false);
    ssc.e = Owner;
    handleApi.SetLoop(ssc.handle, entry.loop);
    handleApi.SetCallback(ssc.handle, &playbackWatcher, Owner.GetIndex());
    //handleApi.SetSourcePositionMode(ssc.handle, entry.positionMode);
}

void SoundSubsystem::OnPlaybackFinished(SoundHandle handle, UserData userData) {
    auto *ssc = componentArray->QuerryComponent<SoundSourceComponent>(userData);
    if (ssc)
        ssc->finished = true;
}

void SoundSubsystem::OnPlaybackLoop(SoundHandle handle, UserData userData) {
    auto *ssc = componentArray->QuerryComponent<SoundSourceComponent>(userData);
    if (ssc)
        ssc->looped = true;
}

}
