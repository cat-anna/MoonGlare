#pragma once

#include "readonly_file_system.h"
#include <string>
#include <string_view>
#include <vector>

namespace MoonGlare {

class iWritableFileSystem : public iReadOnlyFileSystem {
public:
    virtual bool WriteFileByPath(std::string_view path, const std::string &file_data) = 0;

protected:
    virtual ~iWritableFileSystem() = default;
};

} // namespace MoonGlare