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

        namespace Handle {
            enum {
                GenerationBits = 16,
                IndexBits = 12,
                TypeBits = 4,
                ValueBits = 0,

                GenerationLimit = 1 << GenerationBits,
                IndexLimit = 1 << IndexBits,
                TypeLimit = 1 << TypeBits,

                EntryCheckPerStep = 8,
            };

            struct Types {
                enum {
                    Invalid,
                    Object,
                };
            };
        };

        namespace HandleType {
            enum e : uint8_t {
                Invalid,
                Entity,
                Component,
                Resource,
                StaticResource,
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
using Handle = MoonGlare::Component::Handle;

using EntityNameHash = uint32_t;

using HandleIndex = uint16_t;
using HandleType = uint16_t;
using HandleSet = std::vector<Handle>;

} //namespace MoonGlare

inline std::ostream& operator<<(std::ostream &o, MoonGlare::Entity e) {
    char buf[128];
    sprintf_s(buf, "(Entity; Index:%d; Type:%d; Generation:%d)", e.GetIndex(), e.GetType(), e.GetGeneration());
    return o << buf;
}

inline std::ostream& operator<<(std::ostream &o, MoonGlare::Handle h) {
    char buf[128];
    sprintf_s(buf, "(Handle; Index:%d; Type:%d; Generation:%d)", h.GetIndex(), h.GetType(), h.GetGeneration());
    return o << buf;
}

#ifdef LUABRIDGE_LUABRIDGE_HEADER

namespace luabridge {

template <>
struct Stack <MoonGlare::Handle> {
    static void push(lua_State* L, MoonGlare::Handle h) {
        lua_pushlightuserdata(L, h.GetVoidPtr());
    }
    static MoonGlare::Handle get(lua_State* L, int index) {
        return MoonGlare::Handle::FromVoidPtr(lua_touserdata(L, index));
    }
};

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
