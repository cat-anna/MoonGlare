#include <pch.h>
#include <MoonGlare.h>
#include <Engine/ModulesManager.h>
#include <Engine/Core/Engine.h>
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

struct SettingsImpl {
    template<class T>
    void RegisterDynamicSetting(const char *Name, SettingsGroup Group, unsigned Flags) {
        struct V {
            static std::unique_ptr<SettingManipulatorBase> func() { return std::make_unique<T>(); }
        };
        SettingsHandlerInfo value{ Name, Group, &V::func };
        RegisterDynamicSetting(value, Flags);
    }

    struct InternalSettings {
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

    SettingsImpl() {
        //Register<InternalSettings::Localization::Code>("Localization.Code", SettingsGroup::Localization);
        GabiLib::Serialize::DefaultSetter def;
        Settings.Serialize(def);
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

        Settings.Serialize(reader);
        GetModulesManager()->LoadSettings(root.child("EngineModules"));
    }

    void Save() {
        xml_document xml;
        auto root = xml.append_child("Settings");
        GabiLib::Serialize::PugiWritterSerializer writter(root);
        Settings.Serialize(writter);

        GetModulesManager()->SaveSettings(root.append_child("EngineModules"));

        xml.save_file("Settings.xml");
    }

    static void RegisterScriptApi(ApiInitializer &api);
};

inline SettingsImpl* GetImpl() {
    static SettingsImpl *_Impl = nullptr;
    if (!_Impl)
        _Impl = new SettingsImpl();
    return _Impl;
}

//-------------------------------------------------------------------------------------------------

void Settings_t::Load() {
    GetImpl()->Load();
}

void Settings_t::Save() {
    GetImpl()->Save();
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
