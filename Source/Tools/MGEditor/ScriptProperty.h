#pragma once

#include <map>
#include <string>
#include <vector>

#include <ToolBase/Module.h>

namespace MoonGlare::Editor {

struct ScriptPropertySet;

struct ScriptProperty {
    std::string memberName;
    //std::string type;
    std::string comment;
    std::string defaultValue;
    //std::vector<std::string> conditions;
    std::weak_ptr<const ScriptPropertySet> owner;

    bool TestValue(const std::string& value) const {
        __debugbreak();
        return true;
    }
};

struct ScriptPropertyInstance;

struct ScriptPropertySet : public std::enable_shared_from_this<ScriptPropertySet> {
    std::string className;
    std::string parentClassName;

    std::shared_ptr<const ScriptPropertySet> parentPropertySet;
    std::vector<std::shared_ptr<const ScriptProperty>> properties;

    std::vector<std::shared_ptr<const ScriptProperty>> GetAll() const;

    std::shared_ptr<ScriptPropertyInstance> CreateInstance() const;
};

struct ScriptPropertySetInfo{
    std::string className;
    std::string parentClassName;
    std::vector<std::shared_ptr<ScriptProperty>> properties;
};

struct CustomScriptProperty {
    std::string memberName;
};

struct ScriptPropertyInstance {
    std::shared_ptr<const ScriptPropertySet> propertySet;
    std::map<std::shared_ptr<const ScriptProperty>, std::string> values;
    std::unordered_map<std::shared_ptr<CustomScriptProperty>, std::string> customValues;

    std::string Serialize() const;
    void Deseralize(std::string_view values);
};

class ScriptPropertyProvider : public iModule {
public:
    ScriptPropertyProvider(SharedModuleManager modmgr);


    void SetScriptProperties(std::shared_ptr<ScriptPropertySetInfo> info);

    std::shared_ptr<const ScriptPropertySet> GetScriptProperties(const std::string &scriptName);
private:
    std::unordered_map<std::string, std::shared_ptr<ScriptPropertySet>> propertySets;

    std::list<std::weak_ptr<ScriptPropertySet>> orphans;

    void CheckOrphans();
};

}

Q_DECLARE_METATYPE(std::shared_ptr<const MoonGlare::Editor::ScriptProperty>);
Q_DECLARE_METATYPE(std::shared_ptr<MoonGlare::Editor::CustomScriptProperty>);

