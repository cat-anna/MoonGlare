
#include "SoundSourceComponent.h"
#include "SoundSourceSystem.h"
#include "SoundStreamFinishedEvent.h"
#include "SoundSourceComponentLuaWrap.h"

#include "SoundSystemRegister.h"

#include "../iSoundSystem.h"
#include "PlaybackWatcher.h"

namespace MoonGlare::SoundSystem::Component {

void SoundSystemRegister::Register(ECSRegister &r) {
    r.Component<SoundSourceComponent, SoundSourceComponentLuaWrap>();
    //r.System<SoundSourceSystem>("SoundSource");
    r.Event<SoundStreamFinishedEvent>();
}

void SoundSystemRegister::Install(InterfaceMap &ifaceMap) {
    auto ss = iSoundSystem::Create();
    ss->Initialize(ifaceMap);
    ifaceMap.SetSharedInterface(ss);
    auto pw = std::make_shared<Component::PlaybackWatcher>(ifaceMap.GetSharedInterface<Component::EventDispatcher>());
    ss->GetHandleApi().SetCallback(pw);

    SoundSourceComponent::handleApi = ss->GetHandleApi();
    SoundSourceComponentLuaWrap::handleApi = SoundSourceComponent::handleApi;
}

}
