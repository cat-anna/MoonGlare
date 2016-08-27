#include <pch.h>
#include <MoonGlare.h>
#include <Engine/ModulesManager.h>
#include <Engine/iApplication.h>

#include <Utils/LuaUtils.h>

Settings_t Settings;

template <class T, class I>
T EnumCastClamp(I f) {
	T t = static_cast<T>(static_cast<std::underlying_type_t<T>>(f));
	if (t < T::Value_Min)
		return T::Value_Min;
	if (t > T::Value_Max)
		return T::Value_Max;
	return t;
}

//-------------------------------------------------------------------------------------------------

struct SettingsHandler : public Settings_t::SettingsHandlerInfo {
	std::unique_ptr<Settings_t::SettingManipulatorBase> Manipulator;
	unsigned Flags = 0;

	struct Flags {
		enum {
			Modiffied			   = 0x01,
			RequireRestart		   = 0x02,
			Changed				   = 0x04,
			Internal			   = 0x08,
		};
	};
	DefineFlag(Flags, Flags::Modiffied, Modiffied);
	DefineFlag(Flags, Flags::RequireRestart, RequireRestart);
	DefineFlag(Flags, Flags::Changed, Changed);
	DefineFlag(Flags, Flags::Internal, Internal);
};

//-------------------------------------------------------------------------------------------------

struct SettingsImpl {
	using SettingsHandlerInfo = Settings_t::SettingsHandlerInfo;
	using SettingManipulatorBase = Settings_t::SettingManipulatorBase;
	std::unordered_map<string, SettingsHandler> m_DynamicSettings;
	
	template<class T>
	void RegisterDynamicSetting(const char *Name, SettingsGroup Group, unsigned Flags) {
		struct V {
			static std::unique_ptr<SettingManipulatorBase> func() { return std::make_unique<T>(); }
		};
		SettingsHandlerInfo value{ Name, Group, &V::func };
		RegisterDynamicSetting(value, Flags);
	}
	void RegisterDynamicSetting(const SettingsHandlerInfo& handler, unsigned Flags) {
		if (m_DynamicSettings.find(handler.Name) != m_DynamicSettings.end()) {
			AddLog(Error, "Attempt to override setting '" << handler.Name << "'");
			return;
		}
		auto & val = m_DynamicSettings[handler.Name];
		val.Constructor = handler.Constructor;
		val.Flags = Flags;
		val.Name = handler.Name;
		val.NotifyGroup = handler.NotifyGroup;
	}

	struct InternalSettings {
		struct Graphic {
			struct TextureFiltering {
				using Type = int;
				using EnumType = Settings_t::FinteringMode;
				static Type default() { return static_cast<int>(EnumType::Bilinear); }
				static Type get() { return static_cast<int>(Settings.Graphic.Filtering); }
				static void set(Type f) { Settings.Graphic.Filtering = EnumCastClamp<EnumType>(f); }
			};
			struct ShadowQuality {
				using Type = int;
				using EnumType = Settings_t::ShadowQuality;
				static Type default() { return static_cast<int>(EnumType::Medium); }
				static Type get() { return static_cast<int>(Settings.Graphic.Shadows); }
				static void set(Type f) { Settings.Graphic.Shadows = EnumCastClamp<EnumType>(f); }
			};
		};
		struct Localization {
			struct Code {
				using Type = const char *;
				static Type default() { return "en"; }
				static Type get() { return Settings.Localization.Code.c_str(); }
				static void set(Type v) { Settings.Localization.Code = v; }
			};	
		};
	};
	 
	template < class T > 
	void Register(const char *Name, SettingsGroup Group) {
		RegisterDynamicSetting<Settings_t::DirectSettingManipulator<T>>(Name, Group, SettingsHandler::Flags::Internal);
	}
	template < class T> 
	void RegisterBuffered(const char *Name, SettingsGroup Group) {
		RegisterDynamicSetting<Settings_t::BufferedSettingManipulator<T>>(Name, Group, SettingsHandler::Flags::RequireRestart | SettingsHandler::Flags::Internal);
	}

	SettingsImpl() {
		RegisterBuffered<InternalSettings::Graphic::TextureFiltering>("Graphic.TextureFiltering", SettingsGroup::None);
		RegisterBuffered<InternalSettings::Graphic::ShadowQuality>("Graphic.ShadowsQuality", SettingsGroup::None);
		RegisterBuffered<InternalSettings::Localization::Code>("Localization.Code", SettingsGroup::Localization);
	
		GabiLib::Serialize::DefaultSetter def;
		Settings.Serialize(def);
	}

	SettingManipulatorBase* FindSetting(const char *name) {
		auto it = m_DynamicSettings.find(name);
		if (it == m_DynamicSettings.end()) {
			AddLog(Warning, "There is no setting '" << name << "'");
			return nullptr;
		}
		auto &handler = it->second;
		if (!handler.Manipulator) {
			if (!handler.Constructor)
				return 0;
			handler.Manipulator = handler.Constructor();
		}
		return handler.Manipulator.get();
	}

	static pugi::xml_node find_node(pugi::xml_node node, const char *location, bool canCreate) {
		std::string stdpath = location;
		char *path = (char*)stdpath.c_str();
		do {
			auto len = std::strcspn(path, ".");
			char *name = path;
			if (len > 0) {
				path += len;
				if (*path == '.') {
					*path = 0; 
					++path;
				}
			} else
				break;

			auto n = node.child(name);
			if (!n) {
				if (!canCreate)
					return n;
				node = node.append_child(name);
			} else {
				node = n;
			}
		} while (true);
		return node;
	}

	void Load() {
		xml_document xml;
		xml.load_file("Settings.xml");
		auto root = xml.document_element();
		GabiLib::Serialize::PugiReaderSerializer reader(root);

		for (auto &it : m_DynamicSettings) {
			auto &handler = it.second;
			if (handler.IsInternal())
				continue;
			auto node = find_node(root, handler.Name, false);
			if (!node)
				continue;
			if (!handler.Manipulator) {
				if (!handler.Constructor)
					continue;
				handler.Manipulator = handler.Constructor();
			}
			handler.Manipulator->load(node);
			handler.Manipulator.reset();
			handler.SetChanged(true);
		}
		Settings.Serialize(reader);
	}

	void Save() {
		xml_document xml;
		auto root = xml.append_child("Settings");
		GabiLib::Serialize::PugiWritterSerializer writter(root);
		Settings.Serialize(writter);

		for (auto &it : m_DynamicSettings) {
			auto &handler = it.second;
			if (!handler.IsChanged() || handler.IsInternal())
				continue;
			auto node = find_node(root, handler.Name, true);
			if (!handler.Manipulator) {
				if (!handler.Constructor)
					continue;
				handler.Manipulator = handler.Constructor();
			}
			handler.Manipulator->save(node);
			handler.Manipulator.reset();
		}
		xml.save_file("Settings.xml");
	}

	int SetSettingsValue(lua_State* lua) {
		auto what = Utils::Scripts::Lua_to<const char *>(lua, -2);
		auto it = m_DynamicSettings.find(what);
		if (it == m_DynamicSettings.end()) {
			AddLog(Warning, "There is no setting '" << what << "'");
			return 0;
		}
		auto &handler = it->second;
		if (!handler.Manipulator) {
			if (!handler.Constructor)
				return 0;
			handler.Manipulator = handler.Constructor();
		}
		int r = handler.Manipulator->set(lua, -1);
		handler.SetModiffied(true);
		handler.SetChanged(true);
		if (!handler.IsRequireRestart())
			GetModulesManager()->BroadcastNotification(handler.NotifyGroup);
		return r;
	}

	int SetDefaultValue(lua_State* lua) {
		auto what = Utils::Scripts::Lua_to<const char *>(lua, -1);
		auto it = m_DynamicSettings.find(what);
		if (it == m_DynamicSettings.end()) {
			AddLog(Warning, "There is no setting '" << what << "'");
			return 0;
		}
		auto &handler = it->second;
		if (!handler.Manipulator) {
			if (!handler.Constructor)
				return 0;
			handler.Manipulator = handler.Constructor();
		}
		handler.Manipulator->default();
		handler.SetModiffied(true);
		handler.SetChanged(true);
		if (!handler.IsRequireRestart())
			GetModulesManager()->BroadcastNotification(handler.NotifyGroup);
		return 0;
	}

	int GetSettingsValue(lua_State* lua) {
		auto what = Utils::Scripts::Lua_to<const char *>(lua, -1);
		auto it = m_DynamicSettings.find(what);
		if (it == m_DynamicSettings.end()) {
			AddLog(Warning, "There is no setting '" << what << "'");
			return 0;
		}
		auto &handler = it->second;
		if (!handler.Manipulator) {
			if (!handler.Constructor)
				return 0;
			handler.Manipulator = handler.Constructor();
		}
		return handler.Manipulator->get(lua);
	}

	void DumpSettings() {
		std::ostringstream ss;
		for (auto &it: m_DynamicSettings) {
			auto &handler = it.second;
			if (!handler.Manipulator) {
				if (!handler.Constructor)
					continue;
				handler.Manipulator = handler.Constructor();
			}
			ss << handler.Name << " = ";
			handler.Manipulator->dump(ss);
			ss << "\n";
		}
		AddLog(Debug, "Settings dump:\n" << ss.str());
	}

	void ApplySettings() {
		bool restart = false;
		for (auto &it: m_DynamicSettings) {
			auto &handler = it.second;
			if (handler.IsModiffied()) {
				if (!restart && handler.IsRequireRestart())
					restart = true;
				if (handler.Manipulator)
					handler.Manipulator->write();
				it.second.SetModiffied(false);
				it.second.SetChanged(true);
			}
			it.second.Manipulator.reset();
		}
		if (restart) {
			GetApplication()->SetRestart(true);
			MoonGlare::Core::GetEngine()->Exit();
		}
	}

	void CancelSettings() {
		for (auto &it: m_DynamicSettings) {
			auto &handler = it.second;
			if (handler.IsModiffied() && handler.Manipulator) {
				handler.Manipulator->reset();
				it.second.SetModiffied(false);
				it.second.SetChanged(true);
			}
			it.second.Manipulator.reset();
			it.second.SetModiffied(false);
		}
		GetApplication()->SetRestart(false);
	}

	static void RegisterScriptApi(ApiInitializer &api);
};

inline SettingsImpl* GetImpl() {
	static SettingsImpl *_Impl = nullptr;
	if (!_Impl)
		_Impl = new SettingsImpl();
	return _Impl;
}

void SettingsImpl::RegisterScriptApi(ApiInitializer &api) {
	struct T {
		static int SetSettingsValue(lua_State* lua) { return GetImpl()->SetSettingsValue(lua); }
		static int GetSettingsValue(lua_State* lua) { return GetImpl()->GetSettingsValue(lua); }
		static int SetDefaultValue(lua_State* lua) { return GetImpl()->SetDefaultValue(lua); }
		static void DumpSettings() { return GetImpl()->DumpSettings(); }
		static void ApplySettings() { return GetImpl()->ApplySettings(); }
		static void CancelSettings() { return GetImpl()->CancelSettings(); }
	};
	api
	.addCFunction("Set", &T::SetSettingsValue)
	.addFunction("SetDefault", &T::SetDefaultValue)
	.addCFunction("Get", &T::GetSettingsValue)
	.addFunction("Apply", &T::ApplySettings)
	.addFunction("Cancel", &T::CancelSettings)
#ifdef DEBUG_SCRIPTAPI
	.addFunction("Dump", &T::DumpSettings)
#endif
	;
}

RegisterApiNonClass(SettingsImpl, &SettingsImpl::RegisterScriptApi, "Settings");

//-------------------------------------------------------------------------------------------------

void Settings_t::Load() {
	GetImpl()->Load();
}

void Settings_t::Save() {
	GetImpl()->Save();
}

void Settings_t::RegisterDynamicSetting(const SettingsHandlerInfo& handler, bool RequireRestart ) {
	GetImpl()->RegisterDynamicSetting(handler, RequireRestart ? SettingsHandler::Flags::RequireRestart : 0);
}

Settings_t::SettingManipulatorBase* Settings_t::FindSetting(const char *name) {
	return GetImpl()->FindSetting(name);
}

Settings_t::Settings_t() {
}

Settings_t::~Settings_t() {
}

//-------------------------------------------------------------------------------------------------

void Settings_t::GetStdModuleList(std::vector<string> &table) {
	std::vector < string > {
		"./source/DataModules/internal",
#ifdef DEBUG
		"./source/DataModules/debug",
#endif
	}.swap(table);
}

//-------------------------------------------------------------------------------------------------
