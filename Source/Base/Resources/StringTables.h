#pragma once

#include <string>
#include <string_view>
#include <map>

namespace MoonGlare {
class iFileSystem;
}

namespace MoonGlare::Resources {

class StringTables {
public:
    StringTables(iFileSystem *fs);
    virtual ~StringTables();

    const std::string_view GetString(const std::string& id, const std::string& Table) { return GetString(std::string_view(id), std::string_view(Table)); }
    const std::string_view GetString(const std::string_view id, const std::string_view Table);

    void SetLangCode(std::string code);
    void Clear();
    //static void RegisterScriptApi(ApiInitializer &api);
protected:
    iFileSystem *fileSystem = nullptr;
    std::string langCode;
    struct StringTableInfo;

    using StringTableMap = std::map < std::string, StringTableInfo, std::less<> > ;
    StringTableMap m_TableMap;

    void InitInternalTable();

    bool Load(std::string_view TableName);
};

} //namespace MoonGlare::Resources
