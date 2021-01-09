#pragma once

#include <xxhash.h>

namespace MoonGlare {

using FileResourceId = XXH64_hash_t;
static constexpr FileResourceId kInvalidResourceId = 0;

} // namespace MoonGlare
