#pragma once

#include <Libs/libSpace/src/Memory/Handle.h>

#include "Configuration.h"

namespace MoonGlare::Component {

using Entity = Space::Memory::DoubleHandle32<Configuration::EntityIndexBitCount>;

struct EntityHasher {
    std::size_t operator()(const Entity& k) const {
        return std::hash<void*>()(k.GetVoidPtr());
    };
};

using EntityNameHash = uint32_t;

enum class SubSystemInstance : uint32_t { Invalid = 0, };

//Component [instance] index
using ComponentIndex = enum : uint32_t { //??
    Zero = 0,
    Invalid = 0xFFFFFFFF,
};

enum class SubSystemId : uint8_t {
    Invalid = 0x00,
    //SubSystemManager = 0x01,

//Core 
    Script = 0x11,
    Transform = 0x12,

//Renderer
    Mesh = 0x21,
    Light = 0x22,
    Camera = 0x23,
    //DirectAnimation = 0x2F,

//Physics
    Body = 0x50,
    BodyShape = 0x51,

//GUI
    RectTransform = 0x60,
    Image = 0x61,
    Panel = 0x62,
    Text = 0x63,

//Other
    SoundSource = 0x70,
    SoundListener = 0x71,
};

template<SubSystemId VALUE>
struct SubSystemIdWrap {
    constexpr static SubSystemId GetSubSystemId() { return VALUE; };
};

}

#ifdef LUABRIDGE_LUABRIDGE_HEADER

namespace luabridge {

template <>
struct Stack <MoonGlare::Component::Entity> {
    static void push(lua_State* L, MoonGlare::Component::Entity h) {
        lua_pushlightuserdata(L, h.GetVoidPtr());
    }
    static MoonGlare::Component::Entity get(lua_State* L, int index) {
        return MoonGlare::Component::Entity::FromVoidPtr(lua_touserdata(L, index));
    }
};

}

#endif