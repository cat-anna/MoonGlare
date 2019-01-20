#include "AppConfig.h"

namespace MoonGlare {

AppConfig::AppConfig(SharedModuleManager modmgr)
    : iModule(modmgr)
{
}

std::string AppConfig::Get(const std::string& key, const std::string default) const
{
    auto it = values.find(key);
    if (it == values.end())
        return default;
    return it->second;
}

void AppConfig::Set(const std::string& key, std::string value)
{
    values[key] = std::move(value);
}

bool AppConfig::Exists(const std::string& key) const
{
    return values.find(key) != values.end();
}

}
