

#include "tool_base_qt_module_registration.h"
#include "runtime_modules/changes_manager_module.hpp"
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
}

void RegisterBaseQtImporterModules() {
}

} // namespace MoonGlare::Tools
