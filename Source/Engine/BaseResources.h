#pragma once

namespace MoonGlare::Configuration {

struct BaseResources {
	struct FallbackLoadScene : public Settings_t::BaseSettingInfo<std::string, FallbackLoadScene> {
		using Type = std::string;
		static constexpr const char * default() { return "file:///Base/EngineLoadScene.sdx"; }
	};
	struct FirstScene : public Settings_t::BaseSettingInfo<std::string, FirstScene> {
		using Type = std::string;
		static constexpr const char * default() { return "file:///Scenes/MainMenu.sdx"; }
	};
};

} //namespace MoonGlare::Configuration
