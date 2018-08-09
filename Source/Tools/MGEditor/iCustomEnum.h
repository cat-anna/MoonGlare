#pragma once

#include "Module.h"

namespace MoonGlare {
namespace QtShared {

class iCustomEnum {
public:
    virtual ~iCustomEnum() {}

    struct EnumValue {
        std::string caption;
    };

    virtual std::string GetEnumTypeName() const { return ""; }
    virtual std::vector<EnumValue> GetValues() const {
        return{};
    }
};

struct SetEnum : public QtShared::iCustomEnum {
    SetEnum(std::string type) : type(type) {}

    virtual std::string GetEnumTypeName() const {
        return type;
    }
    virtual std::vector<EnumValue> GetValues() const {
        std::vector<EnumValue> ret;       
        {
            std::lock_guard<std::mutex> lock(mutex);
            for (auto &s : set)
                ret.emplace_back(EnumValue{ s });
        }
        return std::move(ret);
    }

    void Add(std::string value) {
        std::lock_guard<std::mutex> lock(mutex);
        set.insert(std::move(value));
    }
    void Remove(std::string value) {
        std::lock_guard<std::mutex> lock(mutex);
        set.erase(std::move(value));
    }
private:
    mutable std::mutex mutex;
    std::set<std::string> set;
    std::string type;
};
using SharedSetEnum = std::shared_ptr<SetEnum>;


class CustomEnumProvider;

class iCustomEnumSupplier {
public:
    virtual ~iCustomEnumSupplier() {}
    virtual std::vector<std::shared_ptr<iCustomEnum>> GetCustomEnums() const { return {}; };
    virtual std::vector<std::shared_ptr<iCustomEnum>> GetCustomEnums(CustomEnumProvider *provider) {
        return GetCustomEnums();
    }
};

class CustomEnumProvider : public iModule {
public:
    CustomEnumProvider(SharedModuleManager modmgr);
    bool PostInit() override;

    template<typename EnumType = SetEnum>
    std::shared_ptr<EnumType> CreateEnum(const std::string &Typename) {
        auto base = std::const_pointer_cast<iCustomEnum>(GetEnum(Typename));
        std::shared_ptr<EnumType> wanted;
        if (!base) {
            wanted = std::make_shared<EnumType>(Typename);
            RegisterEnum(wanted);
        }
        else {
            wanted = std::dynamic_pointer_cast<EnumType>(base);
            if (!wanted)
                return nullptr;
        }

        return wanted;
    }

    const std::shared_ptr<iCustomEnum> GetEnum(const std::string &Typename) const;
private:
    std::unordered_map<std::string, std::shared_ptr<iCustomEnum>> enumMap;

    void RegisterEnum(std::shared_ptr<iCustomEnum> e);
};

} //namespace QtShared 
} //namespace MoonGlare 

