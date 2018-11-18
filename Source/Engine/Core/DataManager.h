#pragma once

#include "DataClasses/Font.h"

namespace MoonGlare {
namespace Core {
namespace Data {

class Manager {
    friend class DataManagerDebugScritpApi;
public:
    Manager(World *world);
    ~Manager();

    bool InitModule(StarVFS::Containers::iContainer *Container);

    void InitFonts();

    DataClasses::FontPtr GetFont(const string &Name);

    static void RegisterScriptApi(::ApiInitializer &api);
private:
    World *world;
    std::unordered_map<std::string, DataClasses::FontPtr> m_Fonts;
    
    void LoadInitScript(StarVFS::Containers::iContainer *Container);
};

} // namespace Data
} // namespace Core 
} // namespace MoonGlare 
