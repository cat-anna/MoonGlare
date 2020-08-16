#pragma once

#include "readonly_file_system.h"
#include "svfs/definitions.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace MoonGlare {

class iWritableFileSystem : public iReadOnlyFileSystem {
protected:
    virtual ~iWritableFileSystem() = default;

public:
};

} // namespace MoonGlare