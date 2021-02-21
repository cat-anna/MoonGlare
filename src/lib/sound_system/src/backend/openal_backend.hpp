#pragma once

#include "ibackend.hpp"
#include <memory>
#include <string>
#include <vector>

namespace MoonGlare::SoundSystem::Backend {

class OpenAlBackend final : public iBackend {
public:
    OpenAlBackend();
    ~OpenAlBackend() override;
};

} // namespace MoonGlare::SoundSystem::Backend
