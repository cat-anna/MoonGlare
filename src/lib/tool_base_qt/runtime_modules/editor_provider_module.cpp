
#include "editor_provider_module.hpp"
#include <boost/algorithm/string/case_conv.hpp>

namespace MoonGlare::Tools::RuntineModules {

using EditorActionInfo = EditorProviderModule::EditorActionInfo;

EditorProviderModule::EditorProviderModule(SharedModuleManager modmgr)
    : iModule(std::move(modmgr)) {
}

bool EditorProviderModule::PostInit() {
    for (auto &item : GetModuleManager()->QueryInterfaces<iEditorInfo>()) {
        auto methodlist = item.interface->GetCreateFileMethods();
        create_methods.reserve(methodlist.size());
        for (auto method : methodlist) {
            create_methods.emplace_back(
                EditorActionInfo{item.module, item.module->cast<iEditorFactory>(), method});
        }

        for (auto &method : item.interface->GetOpenFileMethods()) {
            open_methods.emplace_back(
                EditorActionInfo{item.module, item.module->cast<iEditorFactory>(), method});
        }
    }

    std::sort(create_methods.begin(), create_methods.end(),
              [](const EditorActionInfo &a, const EditorActionInfo &b) {
                  return a.file_handle_method.caption < b.file_handle_method.caption;
              });
    std::sort(open_methods.begin(), open_methods.end(),
              [](const EditorActionInfo &a, const EditorActionInfo &b) {
                  return a.file_handle_method.caption < b.file_handle_method.caption;
              });
    return true;
}

bool EditorProviderModule::Finalize() {
    create_methods.clear();
    open_methods.clear();
    return true;
}

std::vector<EditorActionInfo> EditorProviderModule::GetCreateMethods() const {
    return create_methods;
}

std::vector<EditorActionInfo> EditorProviderModule::GetOpenMethods(std::string ext) const {
    if (ext.empty()) {
        return {};
    }

    boost::to_lower(ext);

    std::vector<EditorProviderModule::EditorActionInfo> r;
    for (auto &method : open_methods) {
        if (boost::to_lower_copy(method.file_handle_method.extension) == ext)
            r.emplace_back(method);
    }
    return r;
}

EditorActionInfo EditorProviderModule::FindOpenEditor(std::string ext) const {
    boost::to_lower(ext);
    if (!ext.empty()) {
        for (auto item : GetModuleManager()->QueryInterfaces<iEditorInfo>())
            for (auto &method : item.interface->GetOpenFileMethods())
                if (boost::to_lower_copy(method.extension) == ext) {
                    return EditorActionInfo{item.module, item.module->cast<iEditorFactory>(),
                                            method};
                }
    }
    throw EditorNotFoundException();
}

} // namespace MoonGlare::Tools::RuntineModules
