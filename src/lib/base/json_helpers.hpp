#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>


namespace MoonGlare {

template <typename T> std::string to_json_string(const T &value) {
    nlohmann::json json = value;
    return json.dump();
}

template <typename T> T from_json_string(const std::string &json_string) {
    nlohmann::json source_json = nlohmann::json::parse(json_string);
    return source_json.get<T>();
}

template <typename T> T ReadJsonFromFile(const std::filesystem::path &path) {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open(path, std::ios_base::binary);
    nlohmann::json json;
    file >> json;
    return json.get<T>();
}

template <typename T> void WriteJsonToFile(const std::filesystem::path &path, const T &value) {
    nlohmann::json json = value;
    std::ofstream file;
    file.open(path, std::ios_base::binary | std::ios::out);
    file << nlohmann::json(value);
}

} // namespace MoonGlare