#pragma once

#include <boost/filesystem/string_file.hpp>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <orbit_logger.h>
#include <string>
#include <string_view>

namespace MoonGlare {

template <typename T>
std::string to_json_string(const T &value, bool pretty_print = false) {
    nlohmann::json json = value;
    return json.dump(pretty_print ? 4 : -1);
}

template <typename T>
T from_json_string(const std::string &json_string) {
    nlohmann::json source_json = nlohmann::json::parse(json_string);
    return source_json.get<T>();
}

template <typename T>
T ReadJsonFromFile(const std::filesystem::path &path) {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open(path, std::ios_base::binary);
    nlohmann::json json;
    file >> json;
    return json.get<T>();
}

template <typename T>
void WriteJsonToFile(const std::filesystem::path &path, const T &value, bool pretty_print = false) {
    nlohmann::json json = value;
    std::ofstream file;
    file.open(path, std::ios_base::binary | std::ios::out);
    file << to_json_string(value, pretty_print);
}
template <typename T>
void WriteIfChangedJsonToFile(const std::filesystem::path &path, const T &value,
                              bool pretty_print = false) {
    std::string content;
    if (std::filesystem::is_regular_file(path)) {
        boost::filesystem::load_string_file(path.generic_string(), content);
    }
    auto new_content = to_json_string(value, pretty_print);
    if (new_content == content) {
        return;
    }
    boost::filesystem::save_string_file(path.generic_string(), new_content);
}

template <typename T>
bool try_get_json_optional_child(const nlohmann::json &json, const char *child_name,
                                 std::optional<T> &t, bool log_error = true) {
    if (json.contains(child_name)) {
        try {
            auto &node = json[child_name];
            if (node.is_null()) {
                t = std::nullopt;
            } else {
                t = node.get<T>();
            }
        } catch (const std::exception &e) {
            if (log_error) {
                AddLogf(Error, "try_get_json_child failed: %s", e.what());
            }
            return false;
        }
    } else {
        t = std::nullopt;
    }
    return true;
}

template <typename T>
bool try_get_json_child(const nlohmann::json &json, const char *child_name, T &t,
                        bool log_error = true) {
    if (json.contains(child_name)) {
        try {
            t = json[child_name].get<T>();
        } catch (const std::exception &e) {
            if (log_error) {
                AddLogf(Error, "try_get_json_child failed: %s", e.what());
            }
        }
        return true;
    }
    return false;
}

template <typename T>
bool try_get_json_child_default(const nlohmann::json &json, const char *child_name, T &t,
                                T default_value, bool log_error = true) {
    if (json.contains(child_name)) {
        try {
            t = json[child_name].get<T>();
        } catch (const std::exception &e) {
            if (log_error) {
                AddLogf(Error, "try_get_json_child failed: %s", e.what());
            }
            t = std::move(default_value);
        }
        return true;
    }
    return false;
}

} // namespace MoonGlare