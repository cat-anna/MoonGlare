#pragma once

#include <unordered_map>
#include <string>
#include <variant>

namespace MoonGlare {

struct Settings {

    enum class ApplyMethod {
        DontCare,
        Immediate,
        Restart,
    };

    using ValueVariant = std::variant<nullptr_t, int, float, bool, std::string>;

    struct iChangeCallback {
        virtual ApplyMethod ValueChanged(const std::string &key, Settings* siface) = 0;
    };

    std::string Serialize() const;
    void Deserialize(const std::string &data);

    void SaveToFile(const std::string &fileName) const;
    void LoadFromFile(const std::string &fileName);

    bool HasValue(const std::string &key) const;
    bool Changed() const { return changed; };

    std::string GetString(const std::string &key, const std::string &default = "");
    int GetInt(const std::string &key, int default = 0);
    float GetFloat(const std::string &key, float default = 0);
    bool GetBool(const std::string &key, bool default = false);
    ValueVariant GetValue(const std::string &key);

    ApplyMethod SetValue(const std::string &key, ValueVariant value);
    ApplyMethod SetValue(const std::string &key, std::string value);
    ApplyMethod SetValue(const std::string &key, int value);
    ApplyMethod SetValue(const std::string &key, float value);
    ApplyMethod SetValue(const std::string &key, bool value);

    std::vector<std::string> GetKeys() const;

    void Subscribe(std::shared_ptr<iChangeCallback> cb);
private:
    std::unordered_map<std::string, ValueVariant> values;
    std::vector<std::weak_ptr<iChangeCallback>> callbackList;
    bool changed = false;
    ApplyMethod NotifyChange(const std::string &key);
};

}
