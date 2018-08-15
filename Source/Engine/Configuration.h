#pragma once

#include <Foundation/Component/Entity.h>

namespace MoonGlare {

    namespace Configuration {

        using HashID = uint32_t;
        using RuntimeRevision = uint32_t; //TODO:unsafe

        namespace Entity {
            enum {
                GenerationBits = 14,
                IndexBits = 14,
                TypeBits = 4, 
                ValueBits = 0,

                GenerationLimit = 1 << GenerationBits,
                IndexLimit = 1 << IndexBits,
                TypeLimit = 1 << TypeBits,

                EntryCheckPerStep = 8,
            };

            struct Types {
                enum {
                    //TBD
                    Unknown,
                    Object,
                };
            };
        };

        namespace Storage {
            enum {
                TinyBuffer = 32,
                SmallBuffer = 256,
                MediumBuffer = 1024,
                HugeBuffer = 4096, 

                EntityBuffer = HugeBuffer,
                ComponentBuffer = HugeBuffer,

                MaxComponentCount = TinyBuffer,
            };

            enum {
                CameraComponent = 8,
            };

            struct Static {
                enum {
                    ObjectBuffer = HugeBuffer,
                    EntityStorage = HugeBuffer,
                };
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

using EntityNameHash = uint32_t;

//using HandleIndex = uint16_t;
//using HandleType = uint16_t;

} //namespace MoonGlare

inline std::ostream& operator<<(std::ostream &o, MoonGlare::Entity e) {
    char buf[128];
    sprintf_s(buf, "(Entity; Index:%d; Type:%d; Generation:%d)", e.GetIndex(), e.GetType(), e.GetGeneration());
    return o << buf;
}

#ifdef LUABRIDGE_LUABRIDGE_HEADER

namespace luabridge {

template <>
struct Stack <MoonGlare::Entity> {
    static void push(lua_State* L, MoonGlare::Entity h) {
        lua_pushlightuserdata(L, h.GetVoidPtr());
    }
    static MoonGlare::Entity get(lua_State* L, int index) {
        return MoonGlare::Entity::FromVoidPtr(lua_touserdata(L, index));
    }
};

}

#endif
