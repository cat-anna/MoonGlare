#pragma once

#include <exception>
#include <memory>
#include <runtime_modules.h>
#include <string>
#include <vector>

namespace MoonGlare::Tools::Editor {

struct EditorNotFoundException : public std::exception {};

struct iEditor;
using SharedEditor = std::shared_ptr<iEditor>;

struct iEditorInfo {
    struct FileHandleMethodInfo {
        std::string extension;
        std::string icon;
        std::string caption;
        std::string method_id;
    };

    virtual std::vector<FileHandleMethodInfo> GetCreateFileMethods() const { return {}; }
    virtual std::vector<FileHandleMethodInfo> GetOpenFileMethods() const { return {}; }

    virtual bool IsMultiInstanceAllowed() const { return false; }

protected:
    virtual ~iEditorInfo() = default;
};

struct iEditorFactory {
    struct EditorRequestOptions {
        std::string full_path;
    };

    virtual SharedEditor GetEditor(const iEditorInfo::FileHandleMethodInfo &method,
                                   const EditorRequestOptions &options) {
        return nullptr;
    }

protected:
    virtual ~iEditorFactory() = default;
};

struct iEditor {
    virtual bool Create(const std::string &full_path,
                        const iEditorInfo::FileHandleMethodInfo &method) {
        return false;
    }

    virtual bool OpenData(const std::string &full_path,
                          const iEditorInfo::FileHandleMethodInfo &method) {
        return false;
    }

    virtual bool SaveData() { return false; }
    virtual bool TryCloseData() { return false; }

protected:
    virtual ~iEditor() = default;
};

class iEditorProvider {
public:
    struct EditorActionInfo {
        SharedModule module;
        std::shared_ptr<iEditorFactory> editor_factory;
        iEditorInfo::FileHandleMethodInfo file_handle_method;
    };

    virtual EditorActionInfo FindOpenEditor(std::string ext) const = 0;
    virtual std::vector<EditorActionInfo> GetCreateMethods() const = 0;
    virtual std::vector<EditorActionInfo> GetOpenMethods(std::string ext) const = 0;

protected:
    virtual ~iEditorProvider() = default;
};

} // namespace MoonGlare::Tools::Editor
