#pragma once

#include <string>
#include <unordered_map>

#include "Module.h"

namespace MoonGlare {

class AppConfig : public iModule {
public:
    AppConfig(SharedModuleManager modmgr);

    using MapType = std::unordered_map<std::string, std::string>;

    std::string Get(const std::string &key, const std::string default = {}) const ;
    void Set(const std::string &key, std::string value);

    bool Exists(const std::string &key) const ;

protected:
    MapType values;
};

}
