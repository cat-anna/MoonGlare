

#include "tool_base_qt_module_registration.h"
#include "runtime_modules/basic_icons_provider_module.hpp"
#include "runtime_modules/changes_manager_module.hpp"
#include "runtime_modules/custom_enum_provider_module.hpp"
#include "runtime_modules/custom_type_editor_module.hpp"
#include "runtime_modules/editor_provider_module.hpp"
#include "runtime_modules/file_icon_provider_module.hpp"
#include "runtime_modules/issue_reporter_module.hpp"

namespace MoonGlare::Tools {

void RegisterBaseQtModules() {
    using namespace RuntineModules;
    ModuleClassRegister::Register<ChangesManagerModule> ChangesManagerReg("ChangesManager");
    ModuleClassRegister::Register<IssueReporterModule> IssueReporterReg("IssueReporter");
    ModuleClassRegister::Register<FileIconProviderModule> FileIconProviderReg("FileIconProvider");
    ModuleClassRegister::Register<EditorProviderModule> EditorProviderReg("FileEditorProvider");
    ModuleClassRegister::Register<CustomTypeEditorProviderModule> CustomTypeEditorProviderReg(
        "CustomTypeEditorProvider");
    ModuleClassRegister::Register<CustomEnumProviderModule> CustomEnumProviderReg("CustomEnumProvider");
    ModuleClassRegister::Register<BasicIconsProviderModule> BasicIconsProviderModuleReg("BasicIconsProvider");
}

void RegisterBaseQtImporterModules() {
}

} // namespace MoonGlare::Tools
