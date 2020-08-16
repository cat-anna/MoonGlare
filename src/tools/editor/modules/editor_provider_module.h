#pragma once

#include <editor_provider.h>
#include <runtime_modules.h>

namespace MoonGlare::Tools::Editor::Modules {

class EditorProviderModule : public iModule, public iEditorProvider {
public:
    EditorProviderModule(SharedModuleManager modmgr);
    bool PostInit() override;
    bool Finalize() override;

    //SharedModule GetExtensionHandler(const std::string &ext) const;
    EditorActionInfo FindOpenEditor(std::string ext) const override;
    std::vector<EditorActionInfo> GetCreateMethods() const override;
    std::vector<EditorActionInfo> GetOpenMethods(std::string ext) const override;

public:
    std::vector<EditorActionInfo> create_methods;
    std::vector<EditorActionInfo> open_methods;
    //std::unordered_map<std::string, SharedModule> extension_handlers;
};

} // namespace MoonGlare::Tools::Editor::Modules
