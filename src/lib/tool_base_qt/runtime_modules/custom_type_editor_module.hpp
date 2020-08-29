#pragma once

#include <QWidget>
#include <custom_type_editor.hpp>
#include <runtime_modules.h>
#include <string>
#include <variant_argument_map.hpp>

namespace MoonGlare::Tools::RuntineModules {

class CustomTypeEditorProviderModule : public iCustomTypeEditorProvider, public iModule {
public:
    CustomTypeEditorProviderModule(SharedModuleManager modmgr);
    bool PostInit() override;
    void RegisterTypeEditor(std::shared_ptr<iCustomTypeEditorFactory> factory, const std::string &type_name) override;
    std::shared_ptr<iCustomTypeEditorFactory> GetEditorFactory(const std::string &type_name) override;

private:
    std::unordered_map<std::string, std::shared_ptr<iCustomTypeEditorFactory>> type_factories;
};

} // namespace MoonGlare::Tools::RuntineModules
