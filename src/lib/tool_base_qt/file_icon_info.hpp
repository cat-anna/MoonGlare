
#pragma once

#include <runtime_modules.h>
#include <string>
#include <vector>

namespace MoonGlare::Tools {

class iFileIconInfo {
public:
    virtual ~iFileIconInfo() {}

    struct FileIconInfo {
        std::string ext;
        std::string icon;
    };

    virtual std::vector<FileIconInfo> GetFileIconInfo() const { return {}; }
};

class iFileIconProvider {
public:
    virtual const std::string &GetExtensionIcon(const std::string &ext) const = 0;
    virtual const std::string &GetExtensionIcon(const std::string &ext, const std::string &default) const = 0;

protected:
    virtual ~iFileIconProvider() = default;
};

} // namespace MoonGlare::Tools
