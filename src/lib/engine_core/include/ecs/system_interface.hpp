
#pragma once

#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>

namespace MoonGlare::ECS {

using SystemId = uint8_t;
constexpr size_t kMaxSystems = 32;
constexpr size_t kMaxStepableSystems = 16;

struct EmptySystemConfig {};

inline void to_json(nlohmann::json &j, const EmptySystemConfig &p) {
    j = {};
}
inline void from_json(const nlohmann::json &j, EmptySystemConfig &p) {
}

struct SystemCreateInfo {};

class iSystem {
public:
    virtual ~iSystem() = default;

    // virtual int PushToLua(lua_State *lua, Entity owner) { return 0; };
    // virtual bool Load(ComponentReader &reader, Entity parent, Entity owner) { return true; }
    // virtual bool Create(Entity owner) { return false; }
    // virtual bool PushEntryToLua(Entity owner, lua_State *lua, int &luarets) { return false; }

protected:
};

class BaseSystemInfo;

class iSystemRegister {
public:
    virtual ~iSystemRegister() = default;

    virtual std::unique_ptr<iSystem> CreateSystem(SystemId sys_id, const SystemCreateInfo &data,
                                                  const nlohmann::json &config_node) const = 0;
    virtual std::vector<std::unique_ptr<iSystem>>
    LoadSystemConfiguration(const SystemCreateInfo &data,
                            const nlohmann::json &config_node) const = 0;
    virtual std::vector<BaseSystemInfo *> GetRegisteredSystemsInfo() const = 0;
};

} // namespace MoonGlare::ECS