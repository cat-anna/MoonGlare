#pragma once

#include <Foundation/Component/Entity.h>

namespace MoonGlare {

    namespace Configuration {

        using HashID = uint32_t;
        using RuntimeRevision = uint32_t; //TODO:unsafe

        namespace Storage {
            enum {
                ComponentBuffer = 4096,
                MaxComponentCount = 32,
                CameraComponent = 8,
            };
        };

        namespace Console {
            enum {
                MaxConsoleLines = 20,
#ifdef GLFW_KEY_GRAVE_ACCENT
                ActivateKey = GLFW_KEY_GRAVE_ACCENT,
#endif
            };
        };

        namespace SceneManager {
            static const char* SceneFileExt = ".sdx";
        }

    } //namespace Configuration

using Configuration::HashID;

using Entity = MoonGlare::Component::Entity;


} //namespace MoonGlare

inline std::ostream& operator<<(std::ostream &o, MoonGlare::Entity e) {
    char buf[128];
    sprintf_s(buf, "(Entity; Index:%d; Generation:%d)", e.GetIndex(), e.GetGeneration());
    return o << buf;
}
    