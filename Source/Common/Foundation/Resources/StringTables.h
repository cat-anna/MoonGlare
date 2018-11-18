#pragma once

#include <string>
#include <string_view>
#include <map>

#include <Foundation/InterfaceMap.h>
#include <Foundation/Settings.h>

namespace MoonGlare {
class iFileSystem;
class InterfaceMap;
}

namespace MoonGlare::Resources {

class StringTables {
public:
    StringTables(InterfaceMap &ifaceMap);
    ~StringTables();

    const std::string_view GetString(const std::string& id, const std::string& Table) { return GetString(std::string_view(id), std::string_view(Table)); }
    const std::string_view GetString(const std::string_view id, const std::string_view Table);

    void Clear();
    //static void RegisterScriptApi(ApiInitializer &api);
protected:
    iFileSystem *fileSystem = nullptr;
    std::string langCode;
    struct StringTableInfo;

    using StringTableMap = std::unordered_map<std::string, StringTableInfo>;
    StringTableMap m_TableMap;

    void InitInternalTable();

    bool Load(std::string_view TableName);
};

} //namespace MoonGlare::Resources
