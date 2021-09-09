#pragma once

#include <orbit_logger.h>

namespace MoonGlare::Renderer {

struct ErrorHandler {
    static void RegisterErrorCallback();
    static void CheckError();
};

} // namespace MoonGlare::Renderer
