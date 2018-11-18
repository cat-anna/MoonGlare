#pragma once

namespace MoonGlare::Resources { class StringTables; }

namespace MoonGlare {
namespace Core {
namespace Data {

class Manager {
    friend class DataManagerDebugScritpApi;
public:
    static Manager *s_instance;
    static void DeleteInstance() { delete s_instance; s_instance = nullptr; }

    Manager(World *world);
    virtual ~Manager();

    bool InitModule(StarVFS::Containers::iContainer *Container);

    void InitFonts();

    DataClasses::FontPtr GetConsoleFont();
    DataClasses::FontPtr GetFont(const string &Name);

    void SetLangCode(std::string langCode);

    static void RegisterScriptApi(::ApiInitializer &api);
private:
    World *world;
    std::unordered_map<std::string, DataClasses::FontPtr> m_Fonts;
    
    void LoadInitScript(StarVFS::Containers::iContainer *Container);
};

} // namespace Data
} // namespace Core 

inline MoonGlare::Core::Data::Manager* GetDataMgr() { return MoonGlare::Core::Data::Manager::s_instance; }

} // namespace MoonGlare 
