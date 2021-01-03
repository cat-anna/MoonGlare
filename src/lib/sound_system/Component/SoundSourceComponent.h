#pragma once 

#include <Foundation/Component/ComponentInfo.h>

#include "../HandleApi.h"

namespace MoonGlare::SoundSystem::Component {
using namespace MoonGlare::Component;

enum class PositionMode : uint8_t {
    None,
    RelativeToListener,
    Absolute,
    MaxValue,

    Default = RelativeToListener,
};

struct SoundSourceComponent {
    static constexpr uint32_t ComponentLimit = 1024;

    static constexpr char* ComponentName = "SoundSource";

    //Entity owner;
    SoundHandle soundHandle;
    //PositionMode  positionMode;

    static HandleApi handleApi;
    
    SoundSourceComponent() {
        soundHandle = SoundHandle::Invalid;
    }

    ~SoundSourceComponent() {
        handleApi.Close(soundHandle);
    }

    bool Load(ComponentReader &reader, Entity owner);
};

}
