#pragma once

#include "../HandleApi.h"

#include <Foundation/Component/nfComponent.h>
#include <Foundation/Component/iSubsystem.h>
#include <Foundation/Component/EntityEvents.h>
#include <Foundation/Component/EntityArrayMapper.h>
#include <Foundation/Memory/ArrayIndexLinear.h>

#include "Configuration.h"

namespace MoonGlare::SoundSystem::Component {         
using namespace MoonGlare::Component;

enum class PositionMode : uint8_t {
    None,
    RelativeToListener,
    Absolute,

    Default = RelativeToListener,
};

class SoundSourceComponent : public iSubsystem {
public:
    explicit SoundSourceComponent(iSubsystemManager *subsystemManager);
    virtual ~SoundSourceComponent();

    bool Initialize() override;
    bool Finalize() override;

    void Step(const SubsystemUpdateData &data) override;
    int PushToLua(lua_State *lua, Entity owner) override;
    bool Load(ComponentReader &reader, Entity parent, Entity owner) override;
    bool Create(Entity owner) override;

    void HandleEvent(const EntityDestructedEvent &event);

    ComponentIndex GetComponentIndex(Entity e) const { return values.entityMapper.GetIndex(e); }
    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api);
private:
    class PlaybackWatcher : public SoundSystem::iPlaybackWatcher {
        SoundSourceComponent *sss;
    public:
        PlaybackWatcher(SoundSourceComponent *sss) : sss(sss) {}
        void OnFinished(SoundHandle handle, bool loop, UserData userData) override {
            sss->OnPlaybackFinished(handle, loop, userData);
        }
    };

    struct LuaWrapper;
    struct Values : public Memory::ArrayIndexLinear<ComponentIndex, Configuration::MaxSoundSourceComponentLimit, Values>{
        Array<Entity>           owner;
        Array<SoundHandle>      soundHandle;
        EntityArrayMapper<>     entityMapper;

        HandleApi               handleApi;
        SoundSourceComponent   *component;

        void ClearArrays() {
            entityMapper.Clear();
            owner.fill({});
            soundHandle.fill(SoundHandle::Invalid);
        }
        void SwapValues(ElementIndex a, ElementIndex b) {
            entityMapper.Swap(owner[a], owner[b]);
            std::swap(owner[a], owner[b]);
            std::swap(soundHandle[a], soundHandle[b]);
        }
        void ReleaseElement(ElementIndex e) {
            entityMapper.ClearIndex(owner[e]);
            handleApi.Close(soundHandle[e]);
            soundHandle[e] = SoundHandle::Invalid;
            owner[e] = {};
        }
        void InitElemenent(ElementIndex e) {
        }
    };
    Values values;

    iSubsystemManager *subsystemManager;
    PlaybackWatcher playbackWatcher{ this };

    void OnPlaybackFinished(SoundHandle handle, bool loop, UserData userData);
};

}
