
#pragma once

#include "entity_manager_interface.hpp"
#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>

namespace MoonGlare::ECS {

using SystemId = uint8_t;
constexpr size_t kMaxSystems = 32;
constexpr size_t kMaxStepableSystems = 16;

struct BaseSystemConfig {
    bool active = true;
};

void to_json(nlohmann::json &j, const BaseSystemConfig &p);
void from_json(const nlohmann::json &j, BaseSystemConfig &p);

class ComponentArray;

struct SystemCreateInfo {
    ComponentArray *const component_array; //TODO: switch to interface
    iEntityManager *const entity_manager;
};

class iSystem {
public:
    virtual ~iSystem() = default;
    iSystem(const SystemCreateInfo &create_info, const BaseSystemConfig &config)
        : component_array(create_info.component_array), entity_manager(create_info.entity_manager),
          active(config.active) {}

    // virtual int PushToLua(lua_State *lua, Entity owner) { return 0; };
    // virtual bool Load(ComponentReader &reader, Entity parent, Entity owner) { return true; }
    // virtual bool Create(Entity owner) { return false; }
    // virtual bool PushEntryToLua(Entity owner, lua_State *lua, int &luarets) { return false; }

    bool IsActive() const { return active; };
    void SetActive(bool v) { active = v; };

    void Advance(double delta_time) {
        if (IsActive()) {
            DoStep(delta_time);
        }
    }

    struct RuntimeSystemInfo {
        bool stepable;
    };
    virtual RuntimeSystemInfo GetSystemInfo() const = 0;

protected:
    auto *GetComponentArray() const { return component_array; }
    auto *GetEntityManager() const { return entity_manager; }

    virtual void DoStep(double time_delta){};

private:
    ComponentArray *const component_array;
    iEntityManager *const entity_manager;

    bool active = true;
};

template <typename SystemImpl>
class SystemBase : public iSystem {
public:
    SystemBase(const SystemCreateInfo &create_info, const BaseSystemConfig &config)
        : iSystem(create_info, config) {}
    ~SystemBase() override = default;
    RuntimeSystemInfo GetSystemInfo() const override {
        return RuntimeSystemInfo{
            .stepable = SystemImpl::kStepable,
        };
    }
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