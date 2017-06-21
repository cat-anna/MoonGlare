#pragma once

#include "Scene/Configuration.Scene.h"

namespace MoonGlare::Core {

struct RuntimeConfiguration {
    std::string consoleFont = "Arial";
    Scene::SceneConfiguration scene;
};

} //namespace MoonGlare::Core
