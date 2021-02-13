#pragma once

#include <nlohmann/json.hpp>
#include <renderer/device_context.hpp>
#include <renderer/renderer_configuration.hpp>
#include <string>

namespace MoonGlare {

struct EngineConfiguration {
    Renderer::WindowInfo window;
};

#ifdef WANTS_TYPE_INFO

auto GetTypeInfo(EngineConfiguration *) {
    return AttributeMapBuilder<EngineConfiguration>::Start("EngineConfiguration")
        ->AddField("window", &EngineConfiguration::window);
}

auto GetTypeInfo(Renderer::WindowInfo *) {
    return AttributeMapBuilder<Renderer::WindowInfo>::Start("Renderer::WindowInfo")
        ->AddField("width", &Renderer::WindowInfo::width)
        ->AddField("height", &Renderer::WindowInfo::height)
        ->AddField("monitor", &Renderer::WindowInfo::monitor)
        ->AddField("full_screen", &Renderer::WindowInfo::full_screen);
}

#endif

inline void to_json(nlohmann::json &j, const EngineConfiguration &p) {
    j = {
        {"window", p.window},
    };
}

inline void from_json(const nlohmann::json &j, EngineConfiguration &p) {
    j.at("window").get_to(p.window);
}

} // namespace MoonGlare
