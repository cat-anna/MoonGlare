#pragma once

#include "Scene/iSceneManager.h"

namespace MoonGlare::Core {

struct RuntimeConfiguration {
    std::string consoleFont = "Arial";
    Scene::SceneConfiguration scene;
};

} //namespace MoonGlare::Core
