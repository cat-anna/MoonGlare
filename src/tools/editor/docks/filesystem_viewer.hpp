
#pragma once

#include "filesystem_viewer_preview.hpp"
#include <QStandardItemModel>
#include <dock_window.hpp>
#include <editor_provider.h>
#include <file_icon_info.hpp>
#include <runtime_modules.h>
#include <writable_file_system.h>

// #include "../Notifications.h"

namespace Ui {
class FilesystemViewer;
}

namespace MoonGlare::Tools::Editor::Docks {

struct FileSystemViewerRole {
    enum {
        IsFile = Qt::UserRole + 1,
        SortString,
        FilePathHash,
        FileFullPath,
        // FileURI,
    };
};

class FileSystemViewer : public DockWindow, public iEditor, public iFileSystemViewerPreview {
    Q_OBJECT;

public:
    FileSystemViewer(QWidget *parent, SharedModuleManager smm);
    virtual ~FileSystemViewer();

    bool Create(const std::string &full_path, const iEditorInfo::FileHandleMethodInfo &what) override;

    void SetPreviewEditor(SharedEditor editor) override;

protected:
    // void Clear();
protected slots:
    void ShowContextMenu(const QPoint &);
    void ItemDoubleClicked(const QModelIndex &);
    // void RefreshFilesystem();
    void RefreshTreeView();
    void OpenFileEditor(iEditorProvider::EditorActionInfo editor_action, const std::string &full_path);
    void CreateFile(iEditorProvider::EditorActionInfo editor_action, const std::string &parent_full_path);

private:
    std::unique_ptr<Ui::FilesystemViewer> ui;
    std::unique_ptr<QStandardItemModel> view_model;
    std::shared_ptr<iWritableFileSystem> filesystem;

    std::unordered_map<StarVfs::FilePathHash, QStandardItem *> view_item_mapping;
    // WeakModule m_EditorModule;

    SharedEditor current_preview_editor;

    std::weak_ptr<iFileIconProvider> file_icon_provider;
    std::weak_ptr<iEditorProvider> editor_provider;
};

} // namespace MoonGlare::Tools::Editor::Docks
