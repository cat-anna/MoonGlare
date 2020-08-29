#include "filesystem_viewer.hpp"
#include "modules/filesystem_module.hpp"
#include <QClipboard>
#include <QList>
#include <QMenu>
#include <QStandardItem>
#include <QStringView>
#include <dock_window_info.hpp>
#include <qt_gui_icons.h>
#include <svfs/path_utils.hpp>
#include <ui_filesystem_viewer.h>

// #include <iFileProcessor.h>

namespace MoonGlare::Tools::Editor::Docks {

//-----------------------------------------

struct FileSystemViewerInfo : public DockWindowModule<FileSystemViewer>, public iEditorInfo, public iEditorFactory {

    std::any QueryInterface(const std::type_info &info) override {
        if (info == typeid(iFileSystemViewerPreview))
            return std::dynamic_pointer_cast<iFileSystemViewerPreview>(GetInstance());
        return {};
    }

    FileSystemViewerInfo(SharedModuleManager modmgr) : DockWindowModule(std::move(modmgr)) {
        SetSettingId("FileSystemViewerInfo");
        SetDisplayName(tr("Filesystem"));
        SetShortcut("F4");
    }

    std::vector<FileHandleMethodInfo> GetCreateFileMethods() const override {
        return std::vector<FileHandleMethodInfo>{
            FileHandleMethodInfo{"{DIR}", ICON_16_FOLDER_RESOURCE, "Folder", "{DIR}"}};
    }

    SharedEditor GetEditor(const iEditorInfo::FileHandleMethodInfo &method,
                           const EditorRequestOptions &options) override {
        return std::dynamic_pointer_cast<iEditor>(GetInstance());
    }
};

ModuleClassRegister::Register<FileSystemViewerInfo> FileSystemViewerInfoReg("FileSystemViewer");

//-----------------------------------------

FileSystemViewer::FileSystemViewer(QWidget *parent, SharedModuleManager smm) : DockWindow(parent, smm) {

    file_icon_provider = smm->QueryModule<iFileIconProvider>();
    editor_provider = smm->QueryModule<iEditorProvider>();

    SetSettingId("FileSystemViewer");
    ui = std::make_unique<Ui::FilesystemViewer>();
    ui->setupUi(this);

    filesystem = smm->QueryModule<iWritableFileSystem>();

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this,
            SLOT(ShowContextMenu(const QPoint &)));

    connect(ui->treeView, SIGNAL(doubleClicked(const QModelIndex &)), SLOT(ItemDoubleClicked(const QModelIndex &)));

    auto fs_mod = smm->QueryModule<Modules::FileSystemModule>();
    connect(fs_mod.get(), SIGNAL(Changed()), this, SLOT(RefreshTreeView()));

    view_model = std::make_unique<QStandardItemModel>();
    view_model->setSortRole(FileSystemViewerRole::SortString);

    ui->treeView->setModel(view_model.get());
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeView->setHeaderHidden(true);

    view_model->setHorizontalHeaderItem(0, new QStandardItem("File"));
    ui->treeView->setColumnWidth(0, 200);

    SetPreviewEditor(nullptr);
    connect(ui->buttonCloseRightPanel, &QPushButton::clicked, [this] { SetPreviewEditor(nullptr); });
}

FileSystemViewer::~FileSystemViewer() {
    // m_EditorProvider.reset();
    file_icon_provider.reset();
    view_model.reset();
    ui.reset();
}

bool FileSystemViewer::Create(const std::string &full_path, const iEditorInfo::FileHandleMethodInfo &what) {
    QString qname;
    if (!QueryStringInput("Enter new folder name:", qname))
        return false;

    std::string new_path = StarVfs::JoinPath(full_path, qname.toStdString());

    ReportNotImplemented();
    //     return m_FileSystem->CreateDirectory(new_path);

    return true;
}

void FileSystemViewer::ShowContextMenu(const QPoint &point) {
    QMenu menu;
    QModelIndex index = ui->treeView->indexAt(point);
    auto itemptr = view_model->itemFromIndex(index);

    std::string fullpath;
    if (itemptr) {
        fullpath = itemptr->data(FileSystemViewerRole::FileFullPath).toString().toStdString();
    }

    bool folder = !index.data(FileSystemViewerRole::IsFile).toBool();

    auto locked_file_icon_provider = file_icon_provider.lock();
    auto locked_editor_provider = editor_provider.lock();

    if (!fullpath.empty()) {
        if (folder) {
            auto *CreateMenu = menu.addMenu(ICON_16_CREATE_RESOURCE, "Create");

            auto methods = locked_editor_provider->GetCreateMethods();
            for (auto methodmodule : methods) {
                if (methodmodule.editor_factory) {
                    auto &method = methodmodule.file_handle_method;
                    CreateMenu->addAction(QIcon(method.icon.c_str()), method.caption.c_str(),
                                          [this, methodmodule, fullpath]() { CreateFile(methodmodule, fullpath); });
                }
            }
        } else {
            auto methods = locked_editor_provider->GetOpenMethods(StarVfs::GetExtension(fullpath));
            if (methods.size() == 0) {
                menu.addAction("Open")->setEnabled(false);
            } else {
                bool single = methods.size() == 1;

                QMenu *openItem = nullptr;
                if (!single)
                    openItem = menu.addMenu("Open...");

                for (auto &method : methods) {
                    auto action = [this, fullpath, method]() { OpenFileEditor(method, fullpath); };

                    if (single) {
                        auto item = menu.addAction(method.file_handle_method.caption.c_str(), this, action);
                        item->setIcon(QIcon(method.file_handle_method.icon.c_str()));
                    } else {
                        auto item = openItem->addAction(method.file_handle_method.caption.c_str(), this, action);
                        item->setIcon(QIcon(method.file_handle_method.icon.c_str()));
                    }
                }
            }
        }
    }

    menu.addSeparator();

    if (itemptr) {
        menu.addAction("Copy path", [itemptr]() {
            auto q_str = itemptr->data(FileSystemViewerRole::FileFullPath).toString();
            QApplication::clipboard()->setText(q_str);
        });
        // menu.addAction("Copy URI", [itemptr]() {
        //     auto qstr = itemptr->data(FileSystemViewerRole::FileURI).toString();
        //     QClipboard *clipboard = QApplication::clipboard();
        //     clipboard->setText(qstr);
        // });
        // menu.addAction("Copy hash URI", [itemptr]() {
        //     auto qstr = itemptr->data(FileSystemViewerRole::FileHash).toString();
        //     QClipboard *clipboard = QApplication::clipboard();
        //     clipboard->setText(qstr);
        // });
        menu.addSeparator();
    }

    menu.addAction(ICON_16_REFRESH, "Refresh", [this]() {
        ReportNotImplemented();
        // m_FileSystem->Reload();
        // RefreshFilesystem();
        // RefreshTreeView();
    });

    menu.addAction("Collapse all", [this]() { ui->treeView->collapseAll(); });
    menu.addAction("Expand all", [this]() { ui->treeView->expandAll(); });

    menu.exec(ui->treeView->mapToGlobal(point));
}

void FileSystemViewer::ItemDoubleClicked(const QModelIndex &) {
    auto selection = ui->treeView->selectionModel()->currentIndex();
    auto parent = selection.parent();

    auto itemptr = view_model->itemFromIndex(selection);
    if (!itemptr)
        return;

    std::string fullpath = itemptr->data(FileSystemViewerRole::FileFullPath).toString().toStdString();

    try {
        auto editor_action = editor_provider.lock()->FindOpenEditor(StarVfs::GetExtension(fullpath));
        OpenFileEditor(editor_action, fullpath);
    } catch (...) {
    }
}

void FileSystemViewer::CreateFile(iEditorProvider::EditorActionInfo editor_action,
                                  const std::string &parent_full_path) {
    try {
        if (!editor_action.editor_factory) {
            throw std::runtime_error("No editor factory provided");
        }

        std::string file_name;
        if (!QueryStringInput("Enter file name", file_name)) {
            return;
        }

        file_name += editor_action.file_handle_method.extension;

        iEditorFactory::EditorRequestOptions request_options;
        request_options.full_path = StarVfs::JoinPath(parent_full_path, file_name);
        auto editor_ptr = editor_action.editor_factory->GetEditor(editor_action.file_handle_method, request_options);

        if (!editor_ptr) {
            throw std::runtime_error("Editor creation failed");
        }
        if (!editor_ptr->Create(request_options.full_path, editor_action.file_handle_method)) {
            throw std::runtime_error("Open action failed");
        }
    } catch (EditorNotFoundException &) {
        AddLog(Warning, "No associated editor with selected file type!");
        ErrorMessage("No associated editor with selected file type!");
    } catch (std::exception &e) {
        AddLogf(Warning, "Create file failed: %s", e.what());
        ErrorMessage(fmt::format("Failed to open file: {}", e.what()));
    }
}

void FileSystemViewer::OpenFileEditor(iEditorProvider::EditorActionInfo editor_action, const std::string &full_path) {
    try {
        if (!editor_action.editor_factory) {
            throw std::runtime_error("No editor factory provided");
        }

        iEditorFactory::EditorRequestOptions request_options;
        request_options.full_path = full_path;
        auto editor_ptr = editor_action.editor_factory->GetEditor(editor_action.file_handle_method, request_options);

        if (!editor_ptr) {
            throw std::runtime_error("Editor creation failed");
        }
        if (!editor_ptr->OpenData(full_path, editor_action.file_handle_method)) {
            throw std::runtime_error("Open action failed");
        }
    } catch (EditorNotFoundException &) {
        AddLog(Warning, "No associated editor with selected file type!");
        ErrorMessage("No associated editor with selected file type!");
    } catch (std::exception &e) {
        AddLogf(Warning, "Open file failed: %s", e.what());
        ErrorMessage(fmt::format("Failed to open file: {}", e.what()));
    }
}

// void FileSystemViewer::RefreshFilesystem() {
//     if (!m_Module) {
//     }
//     m_FileSystem->Reload();
// }

void FileSystemViewer::RefreshTreeView() {
    view_model->clear();

    auto locked_file_icon_provider = file_icon_provider.lock();

    decltype(view_item_mapping) old_mapping = std::move(view_item_mapping);

    std::function<void(std::string, const FileInfoTable::value_type &)> generator;
    generator = [&](std::string parent_path, const FileInfoTable::value_type &file_info) {
        QStandardItem *parent = nullptr;
        QStandardItem *item = nullptr;
        item = old_mapping[file_info.file_path_hash];
        old_mapping.erase(file_info.file_path_hash);

        // std::string n = h.GetName();
        auto full_path = StarVfs::JoinPath(parent_path, file_info.file_name);

        if (!item) {
            parent = old_mapping[file_info.parent_path_hash];
            if (!parent) {
                parent = view_item_mapping[file_info.parent_path_hash];
            }
            if (!parent) {
                parent = view_model->invisibleRootItem();
                view_item_mapping[file_info.parent_path_hash] = parent;
            }

            QList<QStandardItem *> cols;
            cols << (item = new QStandardItem(QString::fromStdString(std::string(file_info.file_name))));
            parent->appendRow(cols);
        }

        view_item_mapping[file_info.file_path_hash] = item;

        if (file_info.is_directory) {
            item->setData(ICON_16_FOLDER, Qt::DecorationRole);
            FileInfoTable result;
            filesystem->EnumeratePath(full_path, result);
            for (auto &item : result) {
                generator(full_path, item);
            }
        } else {
            item->setData(QVariant(true), FileSystemViewerRole::IsFile);
            auto ext = std::string(StarVfs::GetExtension(file_info.file_name));
            if (!ext.empty()) {
                item->setData(QIcon(locked_file_icon_provider->GetExtensionIcon(ext, "").c_str()), Qt::DecorationRole);
            } else {
                item->setData(QIcon(), Qt::DecorationRole);
            }
        }

        // auto str = h.GetFullPath();
        // if (h.IsDirectory() && str.back() != '/')
        //     str.push_back('/');
        // auto hashuri = StarVFS::MakePathHashURI(h.GetHash());
        // auto fileuri = StarVFS::MakeFileURI(str.c_str());
        // item->setData(fileuri.c_str(), FileSystemViewerRole::FileURI);

        item->setData(QString::fromStdString(full_path), FileSystemViewerRole::FileFullPath);
        item->setData(QVariant(file_info.file_path_hash), FileSystemViewerRole::FilePathHash);
        item->setData(QVariant(!file_info.is_directory), FileSystemViewerRole::IsFile);

        std::string sortstring;
        sortstring = file_info.is_directory ? "D" : "F";
        sortstring += file_info.file_name;
        item->setData(sortstring.c_str(), FileSystemViewerRole::SortString);
    };

    FileInfoTable result;
    filesystem->EnumeratePath("", result);
    for (auto &item : result) {
        generator("", item);
    }

    for (auto &it : old_mapping) {
        if (it.second && it.second != view_model->invisibleRootItem()) {
            delete it.second;
        }
    }

    view_model->sort(0);
}

void FileSystemViewer::SetPreviewEditor(SharedEditor editor) {
    if (current_preview_editor) {
        ui->verticalLayout_2->removeWidget(dynamic_cast<QWidget *>(current_preview_editor.get()));
        current_preview_editor.reset();
    }
    QWidget *w = dynamic_cast<QWidget *>(editor.get());
    if (!w) {
        if (editor) {
            AddLogf(Error, "Editor is not a QWidget");
        }

        ui->rightPanel->setVisible(false);
        return;
    }

    w->setParent(this);
    ui->verticalLayout_2->addWidget(w);
    current_preview_editor = editor;
    ui->rightPanel->setVisible(true);
}

} // namespace MoonGlare::Tools::Editor::Docks
