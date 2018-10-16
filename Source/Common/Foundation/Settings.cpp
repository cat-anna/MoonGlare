#include <fstream>
#include <regex>
#include <boost/algorithm/string.hpp>

#include <Foundation/OS/File.h>

#include "Settings.h"

namespace MoonGlare {

Settings::Settings() { }

Settings::Settings(std::shared_ptr<Settings> upperLayerSettings) :upperLayer(std::move(upperLayerSettings)){

}

Settings::~Settings() {}

//---------------------------------------------------------

std::string Settings::Serialize(bool FollowLayers) const {
    std::stringstream ss;
    for (auto &key : GetKeys()) {
        auto it = values.find(key);
        ss << key << "=";
        std::visit([&ss](const auto &value) {
            using type_t = std::remove_cv_t<std::remove_reference_t<decltype(value)>>;
            if constexpr (std::is_same_v<int, type_t>)
                ss << value;
            else if constexpr (std::is_same_v<float, type_t>)
                ss << value;
            else if constexpr (std::is_same_v<std::string, type_t>)
                ss << value;
            else if constexpr (std::is_same_v<bool, type_t>)
                ss << (value ? "true" : "false");
            else {
                __debugbreak();
            }
        }, it->second);
            
        ss << std::endl;
    }
    if (FollowLayers && upperLayer) {
        ss << "#Upper Layers:\n";
        ss << upperLayer->Serialize(true);
    }
    return ss.str();
}

void Settings::Deserialize(const std::string &data) {
    values.clear();
    changed = false;
    std::vector<std::string> lines;
    boost::split(lines, data, boost::is_any_of("\n"));
    for (auto &line : lines) {
        auto comment = line.find("#");
        if (comment != std::string::npos)
            line.erase(line.begin() + comment, line.end());

        AppendString(std::move(line));
    }
}

void Settings::AppendString(std::string line) {
    boost::trim(line);
    if (line.empty())
        return;
    auto pos = line.find('=');
    if (pos == std::string::npos) {
        values[line] = true;
    } else {
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        boost::trim(key);
        boost::trim(value);

        std::smatch base_match;
        if (value == "true")
            values[key] = true;
        else if (value == "false")
            values[key] = false;
        else if (std::regex_match(value, base_match, std::regex("\\d+")))
            values[key] = strtol(value.c_str(), nullptr, 10);
        else if (std::regex_match(value, base_match, std::regex("\\d+.\\d+")))
            values[key] = strtof(value.c_str(), nullptr);
        else
            values[key] = value;
    }
}
void Settings::SaveToFile(const std::string &fileName) const {
    OS::WriteStringToFile(fileName, Serialize());
}

void Settings::LoadFromFile(const std::string &fileName) {
    std::string data;
    if(OS::GetFileContent(fileName, data))
        Deserialize(data);
}

std::vector<std::string> Settings::GetKeys() const {
    std::vector<std::string> r;
    r.reserve(values.size());
    for (auto &it : values) {
        r.emplace_back(it.first);
    }
    std::sort(r.begin(), r.end());
    return r;
}

//---------------------------------------------------------

std::string Settings::ToString(const ValueVariant &vv) {
    return std::visit([](auto &value) -> std::string {
        using type_t = std::remove_cv_t<std::remove_reference_t<decltype(value)>>;
        if constexpr (std::is_same_v<nullptr_t, type_t>)
            return "";
        else if constexpr (std::is_same_v<int, type_t>)
            return std::to_string(value);
        else if constexpr (std::is_same_v<float, type_t>)
            return std::to_string(value);
        else if constexpr (std::is_same_v< std::string, type_t>)
            return value;
        else if constexpr (std::is_same_v<bool, type_t>)
            return value ? "true" : "false";
        else {
            __debugbreak();
            throw std::runtime_error("reached unreachable code!");
        }
    }, vv);
}

bool Settings::HasValue(const std::string &key) const {
    auto it = values.find(key);
    if (it != values.end())
        return true;
    if (upperLayer)
        return upperLayer->HasValue(key);
    return false;
}

std::string Settings::GetString(const std::string &key, const std::string &default) {
    if (upperLayer && upperLayer->HasValue(key)) {
        return upperLayer->GetString(key, default);
    }
    auto it = values.find(key);
    if (it == values.end()) {
#ifdef DEBUG
        SetValue(key, default);
#endif
        return default;
    }
    return ToString(it->second);
}

int Settings::GetInt(const std::string &key, int default) {
    if (upperLayer && upperLayer->HasValue(key)) {
        return upperLayer->GetInt(key, default);
    }
    auto it = values.find(key);
    if (it == values.end()) {
#ifdef DEBUG
        SetValue(key, default);
#endif
        return default;
    }
    return std::visit([](auto &value) -> int {
        using type_t = std::remove_cv_t<std::remove_reference_t<decltype(value)>>;
        if constexpr (std::is_same_v<nullptr_t, type_t>)
            return 0;
        else if constexpr (std::is_same_v<int, type_t>)
            return value;
        else if constexpr (std::is_same_v<float, type_t>)
            return static_cast<int>(value);
        else if constexpr (std::is_same_v<std::string, type_t>)
            return strtol(value.c_str(), nullptr, 10);
        else if constexpr (std::is_same_v<bool, type_t>)
            return value ? 1 : 0;
        else {
            __debugbreak();
            throw std::runtime_error("reached unreachable code!");
        }
    }, it->second);
}

float Settings::GetFloat(const std::string &key, float default)  {
    if (upperLayer && upperLayer->HasValue(key)) {
        return upperLayer->GetFloat(key, default);
    }
    auto it = values.find(key);
    if (it == values.end()) {
#ifdef DEBUG
        SetValue(key, default);
#endif
        return default;
    }
    return std::visit([](auto &value) -> float {
        using type_t = std::remove_cv_t<std::remove_reference_t<decltype(value)>>;
        if constexpr (std::is_same_v<nullptr_t, type_t>)
            return 0.0;
        else if constexpr (std::is_same_v<int, type_t>)
            return value;
        else if constexpr (std::is_same_v<float, type_t>)
            return value;
        else if constexpr (std::is_same_v<std::string, type_t>)
            return strtof(value.c_str(), nullptr);
        else if constexpr (std::is_same_v<bool, type_t>)
            return value ? 1 : 0;
        else {
            __debugbreak();
            throw std::runtime_error("reached unreachable code!");
        }
    }, it->second);
}

bool Settings::GetBool(const std::string &key, bool default) {
    if (upperLayer && upperLayer->HasValue(key)) {
        return upperLayer->GetBool(key, default);
    }
    auto it = values.find(key);
    if (it == values.end()) {
#ifdef DEBUG
        SetValue(key, default);
#endif
        return default;
    }
    return std::visit([](auto &value) -> bool {
        using type_t = std::remove_cv_t<std::remove_reference_t<decltype(value)>>;
        if constexpr (std::is_same_v<nullptr_t, type_t>) 
            return false;
        else if constexpr (std::is_same_v<int, type_t>)
            return value > 0;
        else if constexpr (std::is_same_v<float, type_t>)
            return value != 0.0f;
        else if constexpr (std::is_same_v<std::string, type_t>)
            return value == "true";
        else if constexpr (std::is_same_v<bool, type_t>)
            return value;
        else {
            __debugbreak();
            throw std::runtime_error("reached unreachable code!");
        }
    }, it->second);
}

Settings::ValueVariant Settings::GetValue(const std::string &key) {
    if (upperLayer && upperLayer->HasValue(key)) {
        return upperLayer->GetValue(key);
    }
    auto it = values.find(key);
    if (it == values.end()) {
        return nullptr;
    }
    return it->second;
}

Settings::ApplyMethod Settings::SetValue(const std::string &key, ValueVariant value) {
    if (std::holds_alternative<nullptr_t>(value))
        values.erase(key);
    else
        values[key] = std::move(value);
    changed = true;
    return NotifyChange(key);
}

Settings::ApplyMethod Settings::SetValue(const std::string &key, int value) {
    values[key] = value;
    changed = true;
    return NotifyChange(key);
}

Settings::ApplyMethod Settings::SetValue(const std::string &key, float value) {
    values[key] = value;
    changed = true;
    return NotifyChange(key);
}

Settings::ApplyMethod Settings::SetValue(const std::string &key, bool value) {
    values[key] = value;
    changed = true;
    return NotifyChange(key);
}

Settings::ApplyMethod Settings::SetValue(const std::string &key, std::string value) {
    values[key] = std::move(value);
    changed = true;
    return NotifyChange(key);
}

//---------------------------------------------------------

void Settings::Subscribe(std::shared_ptr<Settings::iChangeCallback> cb) {
    callbackList.emplace_back(cb);
}

Settings::ApplyMethod Settings::NotifyChange(const std::string &key) {    
    AddLogf(Info, "Settings changed key:%s value:%s", key.c_str(), GetString(key).c_str());
    ApplyMethod r = ApplyMethod::DontCare;
    for (auto &wcb : callbackList) {
        auto cb = wcb.lock();
        if (!cb)
            continue;
        switch (cb->ValueChanged(key, this)) {
        case ApplyMethod::Restart:
            r = ApplyMethod::Restart;
            break;
        case ApplyMethod::Immediate:
            if (r != ApplyMethod::Restart)
                r = ApplyMethod::Immediate;
            continue;
        case ApplyMethod::DontCare:
        default:
            continue;
        }
    }
    if (r == ApplyMethod::DontCare)
        return ApplyMethod::Restart;
    return r;
}

}
