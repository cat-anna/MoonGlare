#include "resources/string_tables.hpp"
// #include <interface_map.h>
// #include "iFileSystem.h"

namespace MoonGlare::Resources {

#if 0

struct StringTables::StringTableInfo {
    std::unordered_map<std::string, std::string> StringMap;
    XMLFile XMLTableTranslation;
    XMLFile XMLTable;
};

//------------------------------------------------------------------------------------------

StringTables::StringTables(InterfaceMap &ifaceMap) {
    ifaceMap.GetObject(fileSystem);

    Settings *settings = nullptr;
    ifaceMap.GetObject(settings);
    langCode = settings->GetString("Localization.LangCode", "en");

    Clear();
}

StringTables::~StringTables() {}

//----------------------------------------------------------------------------------

void StringTables::InitInternalTable() {
    auto &mgt = m_TableMap["MoonGlareInfo"].StringMap;
    mgt["InfoLine"] = "?";  // fmt::format("{} {}", Core::ApplicationName, Core::VersionString);
    mgt["BuildDate"] = "?"; // Core::CompilationDate;
    mgt["Version"] = "?";   // Core::VersionString;
}

bool StringTables::Load(std::string_view TableName) {
    XMLFile TableFile, TableTranslationFile;
    char buf[256];
    if (!langCode.empty()) {
        sprintf(buf, "file:///Tables/%s.%s.xml", TableName.data(), langCode.c_str());
        if (!fileSystem->OpenXML(TableTranslationFile, buf)) {
            AddLogf(Warning, "Unable to load translation string table xml file! Table: '%s' File: '%s'",
                    TableName.data(), buf);
        }
    }
    sprintf(buf, "file:///Tables/%s.xml", TableName.data());
    if (fileSystem->OpenXML(TableFile, buf)) {
        AddLogf(Debug, "Unable to load string table xml file! Table: '%s' File: '%s'", TableName.data(), buf);
    }

    if (!TableFile && !TableTranslationFile) {
        AddLogf(Error, "Unable to load string table xml files! Table: '%s'", TableName.data());
        return false;
    }

    auto &table = m_TableMap[TableName.data()];
    table.XMLTable.swap(TableFile);
    table.XMLTableTranslation.swap(TableTranslationFile);
    table.StringMap.clear();
    return true;
}

void StringTables::Clear() {
    m_TableMap.clear();
    InitInternalTable();
}

//------------------------------------------------------------------------------------------

static const std::string_view __NoTable("{Unable to load string table}");
static const std::string_view __NoString("{String does not exists}");
static const std::string_view __EmptyString("");

const std::string_view StringTables::GetString(const std::string_view id, const std::string_view Table) {
    if (id.empty()) {
        return __EmptyString;
    }
    auto tableit = m_TableMap.find(Table.data());
    if (tableit == m_TableMap.end()) {
        if (!Load(Table)) {
            AddLogf(Error, "Unable to load table string '%s' to read string '%s'", Table.data(), id.data());
            return __NoTable;
        }
        return GetString(id, Table);
    }

    auto &tableinfo = tableit->second;
    auto stringit = tableinfo.StringMap.find(id.data());
    if (stringit == tableinfo.StringMap.end()) {
        std::string_view Result;
        auto LookupTable = [id, &Result](XMLFile &file) -> bool {
            if (!file)
                return false;
            auto tablenode = file->document_element();
            auto stringnode = tablenode.find_child_by_attribute("Id", id.data());
            if (!stringnode)
                return false;

            Result = stringnode.text().as_string(__NoString.data());
            return true;
        };
        do {
            if (LookupTable(tableinfo.XMLTableTranslation))
                break;
            if (LookupTable(tableinfo.XMLTable))
                break;
            AddLogf(Error, "String '%s' does not exists in table '%s'", id.data(), Table.data());
            Result = __NoString;
        } while (0);
        tableinfo.StringMap[id.data()] = Result;
        return Result;
    }
    return stringit->second;
}

#endif

} // namespace MoonGlare::Resources
