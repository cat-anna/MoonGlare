
#pragma once

#include "entity_manager_interface.hpp"
#include "renderer/facade.hpp"
#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>

namespace MoonGlare::ECS {

using SystemId = uint8_t;
using SystemOrder = int;
constexpr size_t kMaxSystems = 32;
constexpr size_t kMaxStepableSystems = 16;

struct BaseSystemConfig {
    bool active = true;
};

void to_json(nlohmann::json &j, const BaseSystemConfig &p);
void from_json(const nlohmann::json &j, BaseSystemConfig &p);

class ComponentArray;

struct SystemCreateInfo {
    ComponentArray *const component_array = nullptr; //TODO: switch to interface
    iEntityManager *const entity_manager = nullptr;
    Renderer::iFrameSink *const frame_sink = nullptr;
    Renderer::iResourceManager *const res_manager = nullptr;
};

class iSystem {
public:
    virtual ~iSystem() = default;
    iSystem(const SystemCreateInfo &create_info, const BaseSystemConfig &config)
        : active(config.active) {}

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
        SystemOrder order;
        SystemId id;
    };
    virtual RuntimeSystemInfo GetSystemInfo() const = 0;

protected:
    virtual void DoStep(double time_delta){};

private:
    bool active = true;
};

template <typename SystemImpl>
class SystemBase : public iSystem {
public:
    SystemBase(const SystemCreateInfo &create_info, const BaseSystemConfig &config)
        : iSystem(create_info, config), component_array(create_info.component_array),
          entity_manager(create_info.entity_manager), frame_sink(create_info.frame_sink),
          res_manager(create_info.res_manager) {}
    ~SystemBase() override = default;
    RuntimeSystemInfo GetSystemInfo() const override {
        return RuntimeSystemInfo{
            .stepable = SystemImpl::kStepable,
            .order = SystemImpl::kOrder,
            .id = SystemImpl::kSystemId,
        };
    }

protected:
    auto *GetComponentArray() const { return component_array; }
    auto *GetEntityManager() const { return entity_manager; }
    auto *GetFrameSink() const { return frame_sink; }
    auto *GetResourceManager() const { return res_manager; }

private:
    ComponentArray *const component_array;
    iEntityManager *const entity_manager;
    Renderer::iFrameSink *const frame_sink;
    Renderer::iResourceManager *const res_manager;
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