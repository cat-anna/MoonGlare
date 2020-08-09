#include "iEditor.h"
#include <boost/algorithm/string/case_conv.hpp>

namespace MoonGlare {
namespace QtShared {

ModuleClassRegister::Register<EditorProvider> EditorProviderReg("EditorProvider");

EditorProvider::EditorProvider(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

bool EditorProvider::PostInit() {
    for (auto &item : GetModuleManager()->QueryInterfaces<iEditorInfo>()) {
        auto methodlist = item.m_Interface->GetCreateFileMethods();
        m_CreateMethods.reserve(methodlist.size());
        for (auto method : methodlist) {
            m_CreateMethods.emplace_back(
                EditorActionInfo{item.m_Module, item.m_Module->cast<iEditorFactory>(), method});
        }

        for (auto &method : item.m_Interface->GetOpenFileMethods()) {
            m_OpenMethods.emplace_back(EditorActionInfo{item.m_Module, item.m_Module->cast<iEditorFactory>(), method});
        }
    }

    std::sort(m_CreateMethods.begin(), m_CreateMethods.end(), [](const EditorActionInfo &a, const EditorActionInfo &b) {
        return a.m_FileHandleMethod.m_Caption < b.m_FileHandleMethod.m_Caption;
    });
    std::sort(m_OpenMethods.begin(), m_OpenMethods.end(), [](const EditorActionInfo &a, const EditorActionInfo &b) {
        return a.m_FileHandleMethod.m_Caption < b.m_FileHandleMethod.m_Caption;
    });
    return true;
}

bool EditorProvider::Finalize() {
    m_CreateMethods.clear();
    m_OpenMethods.clear();
    return true;
}

std::vector<EditorProvider::EditorActionInfo> EditorProvider::GetOpenMethods(std::string ext) const {
    boost::to_lower(ext);
    if (ext.front() == '.')
        ext = ext.substr(1);

    std::vector<EditorProvider::EditorActionInfo> r;
    for (auto &method : m_OpenMethods) {
        if (boost::to_lower_copy(method.m_FileHandleMethod.m_Ext) == ext)
            r.emplace_back(method);
    }
    return r;
}

const EditorProvider::EditorActionInfo EditorProvider::FindOpenEditor(std::string ext) {
    boost::to_lower(ext);
    if (!ext.empty()) {
        if (ext.front() == '.')
            ext = ext.substr(1);

        for (auto item : GetModuleManager()->QueryInterfaces<iEditorInfo>())
            for (auto &method : item.m_Interface->GetOpenFileMethods())
                if (boost::to_lower_copy(method.m_Ext) == ext) {
                    return EditorActionInfo{item.m_Module, item.m_Module->cast<iEditorFactory>(), method};
                }
    }
    throw EditorNotFoundException();
}

} // namespace QtShared
} // namespace MoonGlare
