#include PCH_HEADER

#include "ScriptProperty.h"
#include "ScriptPropertyEditor.h"

#include <boost/algorithm/string.hpp>

namespace MoonGlare::Editor {

std::vector<std::shared_ptr<const ScriptProperty>> ScriptPropertySet::GetAll() const {
    std::vector<std::shared_ptr<const ScriptProperty>> r;
    if (parentPropertySet)
        r = parentPropertySet->GetAll();
    r.insert(r.end(), properties.begin(), properties.end());
    return std::move(r);
}

std::shared_ptr<ScriptPropertyInstance> ScriptPropertySet::CreateInstance() const {
    auto r = std::make_shared< ScriptPropertyInstance>();

    r->propertySet = shared_from_this();
    for (auto &item : GetAll()) {
        r->values[item] = item->defaultValue;
    }

    return std::move(r);
}

//-------------------------------------------------------

void ScriptPropertyInstance::Deseralize(std::string_view text) {
    if (text.empty())
        return;

    if (text.front() == '{')
        text.remove_prefix(1);
    if (text.back() == '}')
        text.remove_suffix(1);

    std::list<std::string> lines;
    boost::split(lines, text, boost::is_any_of(","));

    auto getNext = [&]() -> std::tuple<std::string, std::string> {
        std::string key, value;

        while (lines.size() > 0) {
            auto line = lines.front();
            boost::trim(line);
            if (line.empty()) {
                lines.pop_front();
                continue;
            }

            auto eqChar = line.find("=");
            if (eqChar != std::string::npos) {
                if (key.empty()) {
                    key = line.substr(0, eqChar);
                    value = line.substr(eqChar + 1);
                    lines.pop_front();
                    continue;
                } else
                    break;
            }

            value += ",";
            value += line;
            lines.pop_front();
        }

        return { key, value };
    };

    while (lines.size() > 0) {
        auto [key, newValue] = getNext();

        auto findMember = [&key](auto &v) {return v.first->memberName == key; };
        if (auto itProp = std::find_if(values.begin(), values.end(), findMember); itProp != values.end()) {
            itProp->second = newValue;
            continue;
        }
        if (auto itProp = std::find_if(customValues.begin(), customValues.end(), findMember); itProp != customValues.end()) {
            itProp->second = newValue;
            continue;
        }

        auto csp = std::make_shared<CustomScriptProperty>();
        csp->memberName = key;
        customValues[csp] = newValue;
    }
}                            

std::string ScriptPropertyInstance::Serialize() const {
    std::stringstream ss;
    ss << "{";
    for (auto &[item, value] : values) {
        ss << item->memberName << "=" << value << ",";
    }
    for (auto &[item, value] : customValues) {
        ss << item->memberName << "=" << value << ",";
    }
    ss << "}";
    return ss.str();
}   

//-------------------------------------------------------

ScriptPropertyProvider::ScriptPropertyProvider(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {

    TypeEditor::TypeEditorInfo::RegisterTypeEditor(std::make_shared<
        TypeEditor::TemplateTypeEditorInfo<ScriptPropertyEditor>>(), "string:Script.Properties");
}

void ScriptPropertyProvider::SetScriptProperties(std::shared_ptr<ScriptPropertySetInfo> info) {
    auto sps = std::make_shared<ScriptPropertySet>();

    sps->className = info->className;
    sps->parentClassName = info->parentClassName;

    sps->parentPropertySet = GetScriptProperties(sps->parentClassName);
    propertySets[sps->className] = sps;
    AddLogf(Info, "Got script properties for class: %s", info->className.c_str());
    if (!sps->parentPropertySet) {
        orphans.push_back(sps);
        AddLogf(Warning, "Script properties for class: %s has no parent connected", info->className.c_str());
    }

    for (auto &item : info->properties) {
        auto prop = std::make_shared<ScriptProperty>(*item);
        prop->owner = sps;
        sps->properties.emplace_back(prop);
    }

    CheckOrphans();
}

std::shared_ptr<const ScriptPropertySet> ScriptPropertyProvider::GetScriptProperties(const std::string &scriptName) {
    auto it = propertySets.find(scriptName);
    if (it == propertySets.end())
        return nullptr;
    return it->second;
}

void ScriptPropertyProvider::CheckOrphans() {
    std::list<std::weak_ptr<ScriptPropertySet>> orphanList;
    orphanList.swap(orphans);

    for (auto weakItem : orphanList) {
        auto item = weakItem.lock();
        if (!item) {
            //TODO: some error?
            continue;
        }
        auto parent = GetScriptProperties(item->parentClassName);
        if (parent) {
            item->parentPropertySet = parent;
            AddLogf(Info, "Found parent for class: %s", item->className.c_str());
        }
        else {
            orphans.push_back(item);
        }
    }

}

ModuleClassRgister::Register<ScriptPropertyProvider> ScriptPropertyProviderReg("ScriptPropertyProvider");

}