#pragma once

#include <custom_enum_provider.hpp>
#include <custom_type_editor.hpp>
#include <runtime_modules.h>

namespace MoonGlare::Tools::RuntineModules {

class CustomEnumProviderModule : public iCustomEnumProvider, public iModule {
public:
    CustomEnumProviderModule(SharedModuleManager modmgr);
    bool Initialize() override;
    bool PostInit() override;

    void RegisterEnum(std::shared_ptr<iCustomEnum> e, bool wants_type_editor) override;
    std::shared_ptr<iCustomEnum> GetEnum(const std::string &type_name) const override;

private:
    std::unordered_map<std::string, std::shared_ptr<iCustomEnum>> enum_map;
    std::shared_ptr<iCustomTypeEditorProvider> custom_type_editor_provider;
};

} // namespace MoonGlare::Tools::RuntineModules
