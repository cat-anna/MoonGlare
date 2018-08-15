#pragma once

#include <Libs/libSpace/src/Memory/Handle.h>

namespace MoonGlare::Component {

//temp defs
using Entity = Space::Memory::TripleHandle32<15, 16>;

using ComponentIndex = enum : uint32_t {
    Zero = 0,
    Invalid = 0xFFFFFFFF,
};

}
