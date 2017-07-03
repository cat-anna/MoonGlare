/*
 * cModuleManager.h
 *
 *  Created on: 08-12-2013
 *      Author: Paweu
 */

#ifndef CDATAMANAGER_H_
#define CDATAMANAGER_H_

namespace MoonGlare {
namespace Core {
namespace Data {

#define DEFAULT_FONT_NAME	"Arial"

template<class T>
struct SynchronizedResourceMap {
    using Map_t = std::unordered_map < string, T > ;
    struct MapProxy {
        Map_t* operator->() { 
            THROW_ASSERT(m_Map, "Map proxy is not valid!");
            return m_Map; 
        }

        Map_t& operator*() {
            THROW_ASSERT(m_Map, "Map proxy is not valid!");
            return *m_Map; 
        }

        template<class V>
        T& operator[](V &&v) { 
            THROW_ASSERT(m_Map, "Map proxy is not valid!");
            return (*m_Map)[std::forward<V>(v)]; 
        }

        MapProxy(Map_t &map, std::mutex &mutex) : m_Map(&map), m_Lock(mutex) {  }
        MapProxy(MapProxy&& other): m_Map(nullptr), m_Lock() {
            m_Lock.swap(other.m_Lock);
            std::swap(m_Map, other.m_Map);
        }
        MapProxy& operator=(const MapProxy&) = delete;
        MapProxy() = delete;
        ~MapProxy() { }

        void unlock() {
            m_Map = nullptr;
            std::unique_lock<std::mutex> l;
            m_Lock.swap(l);
        }
    private:
        Map_t *m_Map;
        std::unique_lock<std::mutex> m_Lock;
    };

    MapProxy Lock() {
        return MapProxy(m_Map, m_Mutex);
    }
    void clear() { Lock()->clear(); }
private:
    Map_t m_Map;
    std::mutex m_Mutex;
};

struct DataModuleInfo {
    StarVFS::Containers::iContainer *m_Container;
    std::string m_ModuleName;
};

class Manager : public cRootClass {
    friend class DataManagerDebugScritpApi;
    SPACERTTI_DECLARE_CLASS_SINGLETON(Manager, cRootClass)
public:
    Manager(World *world);
    virtual ~Manager();

    bool LoadModule(StarVFS::Containers::iContainer *Container);

    void LoadGlobalData();

    DataClasses::FontPtr GetConsoleFont();
    DataClasses::FontPtr GetDefaultFont();

    DataClasses::FontPtr GetFont(const string &Name);

    const string& GetString(const string &Id, const string& TableName);

    DataClasses::StringTable* GetStringTables() { return m_StringTables.get(); }

    bool Initialize(const std::vector<std::string> &modules, Scripts::ScriptEngine *ScriptEngine);
    bool Finalize();

    void DumpAllResources(std::ostream &out);
    /** Reprint all resources */
#ifdef DEBUG_DUMP
    void NotifyResourcesChanged();
#else
    void NotifyResourcesChanged() { }
#endif

    static void RegisterScriptApi(::ApiInitializer &api);
private:
    Scripts::ScriptEngine *m_ScriptEngine;

    std::vector<DataModuleInfo> m_Modules;
    World *world;

    SynchronizedResourceMap<FontResPtr> m_Fonts;
    
    std::unique_ptr<DataClasses::StringTable> m_StringTables;

    void DumpResources();

    bool LoadModuleScripts(StarVFS::Containers::iContainer *Container);
};

} // namespace Data
} // namespace Core 

inline MoonGlare::Core::Data::Manager* GetDataMgr() { return MoonGlare::Core::Data::Manager::Instance(); }

} // namespace MoonGlare 

#endif // CMODULEMANAGER_H_ 
