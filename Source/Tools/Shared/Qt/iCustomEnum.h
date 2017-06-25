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

class iCustomEnumSupplier {
public:
    virtual ~iCustomEnumSupplier() {}
    virtual std::vector<std::shared_ptr<iCustomEnum>> GetCustomEnums() const = 0;
};

class CustomEnumProvider : public iModule {
public:
    CustomEnumProvider(SharedModuleManager modmgr);
    bool PostInit() override;

    const std::shared_ptr<iCustomEnum> GetEnum(const std::string &Typename) const;
private:
    std::unordered_map<std::string, std::weak_ptr<iCustomEnum>> enumMap;
};

} //namespace QtShared 
} //namespace MoonGlare 

