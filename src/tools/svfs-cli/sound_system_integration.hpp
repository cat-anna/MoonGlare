#pragma once

#include "luainterface.h"
#include <memory>
#include "readonly_file_system.h"

namespace MoonGlare::Tools::VfsCli {

class SoundSystemIntegration {
public:
    SoundSystemIntegration(SharedLua lua, iReadOnlyFileSystem *fs);
    ~SoundSystemIntegration();

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace MoonGlare::Tools::VfsCli