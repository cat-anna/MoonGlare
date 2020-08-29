#pragma once

#include <memory>
#include <mutex>
#include <runtime_modules.h>
#include <string>
#include <unordered_set>
#include <variant_argument_map.hpp>
#include <vector>

namespace MoonGlare::Tools {

class iCustomEnum : public std::enable_shared_from_this<iCustomEnum> {
public:
    virtual ~iCustomEnum() {}

    struct EnumValue {
        std::string caption;
        std::any value;
        bool operator==(const EnumValue &other) const { return caption == other.caption; }
    };

    virtual std::string GetEnumTypeName() const = 0;
    virtual std::vector<EnumValue> GetValues() const = 0;
};

class AliasEnum : public iCustomEnum {
public:
    AliasEnum(std::string type, std::shared_ptr<iCustomEnum> alias_target)
        : type(std::move(type)), alias_target(std::move(alias_target)) {}

    std::string GetEnumTypeName() const override { return type; }
    std::vector<EnumValue> GetValues() const override { return alias_target->GetValues(); }

private:
    std::shared_ptr<iCustomEnum> alias_target;
    std::string type;
};

struct SetEnum : public iCustomEnum {
    SetEnum(std::string type) : type(std::move(type)) {}

    std::string GetEnumTypeName() const override { return type; }
    std::vector<EnumValue> GetValues() const override {
        std::vector<EnumValue> ret;
        {
            std::lock_guard<std::mutex> lock(mutex);
            for (auto &s : set)
                ret.emplace_back(s);
        }
        return std::move(ret);
    }

    void Add(std::string caption, std::any value) {
        std::lock_guard<std::mutex> lock(mutex);
        set.insert(EnumValue{std::move(caption), std::move(value)});
    }
    void Remove(const std::string &caption) {
        std::lock_guard<std::mutex> lock(mutex);
        set.erase(EnumValue{caption});
    }

private:
    mutable std::mutex mutex;
    struct Hasher {
        size_t operator()(const EnumValue &e) const { return std::hash<std::string>()(e.caption); }
    };
    std::unordered_set<EnumValue, Hasher> set;
    std::string type;
};

struct ConstEnum : public iCustomEnum {
    ConstEnum(std::string type, std::vector<EnumValue> values) : type(std::move(type)), values(std::move(values)) {}

    std::string GetEnumTypeName() const override { return type; }
    std::vector<EnumValue> GetValues() const override { return values; }

private:
    std::vector<EnumValue> values;
    std::string type;
};

class iCustomEnumSupplier {
public:
    virtual ~iCustomEnumSupplier() {}
    virtual std::vector<std::pair<bool, std::shared_ptr<iCustomEnum>>> GetCustomEnums() const = 0;
};

class iCustomEnumProvider {
public:
    virtual void RegisterEnum(std::shared_ptr<iCustomEnum> e, bool wants_type_editor = true) = 0;
    virtual std::shared_ptr<iCustomEnum> GetEnum(const std::string &type_name) const = 0;

    void SetAlias(std::string type_name, std::shared_ptr<iCustomEnum> alias_enum) {
        RegisterEnum(std::make_shared<AliasEnum>(std::move(type_name), std::move(alias_enum)));
    }
};

} // namespace MoonGlare::Tools
