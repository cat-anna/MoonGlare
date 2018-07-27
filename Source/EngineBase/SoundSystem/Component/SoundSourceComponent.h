#pragma once

#include "../HandleApi.h"

namespace MoonGlare::SoundSystem::Component {

enum class PositionMode : uint8_t {
    None,
    RelativeToListener,
    Absolute,

    Default = RelativeToListener,
};

struct SoundSourceComponent  {
    SoundState wantedState;
    std::string uri;
    SoundHandle handle;
    bool looped;
    bool finished;
};

}
