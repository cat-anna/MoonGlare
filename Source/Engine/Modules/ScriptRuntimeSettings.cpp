#include <pch.h>
#include <nfMoonGlare.h>
#include <Engine/Application.h>
#include <Core/Scripts/iLuaSettings.h>

#include <Renderer/Configuration.Renderer.h>
#include <Renderer/iContext.h>

#include <AssetSettings.x2c.h>
#include <RendererSettings.x2c.h>
#include <EngineSettings.x2c.h>

#include "iModule.h"
#include <Core/Scripts/ScriptEngine.h>

namespace MoonGlare::Modules {

struct ScriptRuntimeSettings : public MoonGlare::Modules::iModule, public Core::Scripts::Settings::iSettingsProvider {
    //x2c::Settings::EngineSettings_t *settings = nullptr;

    using ValueVariant = Core::Scripts::Settings::ValueVariant;
    using Setting = Core::Scripts::Settings::Setting;
    using InvalidSettingId = Core::Scripts::Settings::iSettingsProvider::InvalidSettingId;

    ScriptRuntimeSettings(World *world) : iModule(world) {}

    void OnPostInit() override {
        auto smod = Core::GetScriptEngine()->QuerryModule<Core::Scripts::Settings::iLuaSettingsModule>();
        smod->RegisterProvider("Core", this);
        //settings = GetWorld()->GetInterface<Application>()->GetConfiguration();
    }

    std::unordered_map<std::string, Setting> GetSettingList(std::string_view prefix) const override {
        using namespace Core::Scripts::Settings;
        switch (Space::Utils::MakeHash32(prefix.data(), prefix.length())) {
        case "Core"_Hash32:
            return{
                { "LangCode", Setting{ ApplyMethod::Restart } },
            };
        default:
            throw InvalidSettingId{};
        }
    }

    template<typename SET, typename M = int, typename T>
    void Set(T &t, ValueVariant vv){ t = static_cast<T>(static_cast<M>(std::get<SET>(vv))); }

    void Set(std::string_view prefix, std::string_view id, ValueVariant value) override {
        auto full = fmt::format("{}.{}", std::string(prefix), std::string(id));
        switch (Space::Utils::MakeHash32(full.c_str())) {
        case "Core.LangCode"_Hash32:
            //settings->m_Core.m_LangCode = std::get<std::string>(value);
            break;
        default:
            throw InvalidSettingId{};
        };
    }
    ValueVariant Get(std::string_view prefix, std::string_view id) override {
        auto full = fmt::format("{}.{}", std::string(prefix), std::string(id));
        switch (Space::Utils::MakeHash32(full.c_str())) {
        case "Core.LangCode"_Hash32:
            return std::string("en");
        default:
            throw InvalidSettingId{};
        };
    }
};

ModuleClassRegister::Register<ScriptRuntimeSettings> ScriptRuntimeSettingsReg("ScriptRuntimeSettings");

} //namespace MoonGlare::Modules
