#pragma once

#include <libSpace/src/Memory/Handle.h>

namespace MoonGlare {

namespace Configuration {

static constexpr size_t HandleIndexBits = 15;
static constexpr size_t HandleIndexLimit = 1 << HandleIndexBits;

static constexpr size_t HandleGenerationBits = 16;
static constexpr size_t HandleGenerationLimit = 1 << HandleGenerationBits;

using Handle = Space::Memory::TripleHandle32<HandleIndexBits, HandleGenerationBits>;

}

using Handle = Configuration::Handle;

}
