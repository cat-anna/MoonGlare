#include <InterfaceMap.h>
#include "SoundSubsystem.h"
#include "SoundSourceComponent.h"

#include "../iSoundSystem.h"

#include <EngineBase/Component/ComponentArray.h>
#include <EngineBase/Component/EventDispatcher.h>

#include "Events.h"

#include <SoundSourceComponent.x2c.h>
                           
namespace MoonGlare::SoundSystem::Component {

HandleApi SoundSourceComponent::handleApi;

//---------------------------------------------------------

SoundSubsystem::SoundSubsystem(iSubsystemManager *subsystemManager)
    : iSubsystem(), subsystemManager(subsystemManager) {

    handleApi = subsystemManager->GetInterfaceMap().GetInterface<iSoundSystem>()->GetHandleApi();
    componentArray = &subsystemManager->GetComponentArray();

    SoundSourceComponent::handleApi = handleApi;
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
            subsystemManager->GetEventDispatcher().SendTo(SoundStreamFinished{ handleApi.GetLoop(ssc.handle) }, ssc.e);
            ssc.finishEvent = false;
        }

        //if (ssc.uriChanged) {
        //    handleApi.Close(ssc.handle, false);
        //    ssc.handle = handleApi.Open(ssc.uri, false, SoundKind::Music, false);
        //    handleApi.SetCallback(ssc.handle, &playbackWatcher, index);
        //    ssc.uriChanged = false;
        //}

        if (ssc.autostart) {
            ssc.autostart = false;
            handleApi.Play(ssc.handle);
        }
    });
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
    ssc.handle = handleApi.Open(entry.uri, false, SoundKind::Music, false);
    handleApi.SetCallback(ssc.handle, &playbackWatcher, Owner.GetIndex());

    return true;
}

void SoundSubsystem::OnPlaybackFinished(SoundHandle handle, bool loop, UserData userData) {
    auto *ssc = componentArray->QuerryComponent<SoundSourceComponent>(userData);
    if (ssc)
        ssc->finishEvent = true;
}

}
