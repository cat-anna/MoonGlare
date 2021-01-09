#pragma once

#include <device_context.hpp>
#include <nlohmann/json.hpp>
#include <renderer_configuration.hpp>
#include <string>

namespace MoonGlare {

struct EngineConfiguration {
    Renderer::WindowInfo window;
};

#ifdef _WANTS_TYPE_INFO_

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

inline void to_json(nlohmann::json &j, const Renderer::Configuration::RendererConfiguration &p) {
    j = {
        // {"value", p.value},
    };
}

inline void from_json(const nlohmann::json &j, Renderer::Configuration::RendererConfiguration &p) {
    // j.at("value").get_to(p.value);
}

inline void to_json(nlohmann::json &j, const EngineConfiguration &p) {
    j = {
        {"window", p.window},
    };
}

inline void from_json(const nlohmann::json &j, EngineConfiguration &p) {
    j.at("window").get_to(p.window);
}

} // namespace MoonGlare
