#pragma once

#include <any>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace MoonGlare::StarVfs {

struct VariantArgumentMap {
    void set(const std::string &key, const std::string &value) { container[key] = value; }
    const std::string &get(const std::string &key, const std::string &default_value) const {
        auto it = container.find(key);
        if (it == container.end()) {
            return default_value;
        } else {
            return it->second;
        }
    }
    const std::string &get(const std::string &key) const {
        auto it = container.find(key);
        if (it == container.end()) {
            throw std::runtime_error("Key " + key + " not found");
        } else {
            return it->second;
        }
    }

private:
    std::unordered_map<std::string, std::string> container;
};

} // namespace MoonGlare::StarVfs
