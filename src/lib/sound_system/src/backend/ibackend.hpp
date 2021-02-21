#pragma once

#include <memory>
#include <string>
#include <vector>

namespace MoonGlare::SoundSystem::Backend {

class iBackend {
public:
    virtual ~iBackend() = default;
};

} // namespace MoonGlare::SoundSystem::Backend
