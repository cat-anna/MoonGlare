
#include "entity_editor.hpp"
#include <qt_gui_icons.h>
#include <ui_entity_editor.h>

// #include "../Windows/MainWindow.h"
// #include <DockWindowInfo.h>
// #include <FileSystem.h>
// #include <icons.h>
// #include <TypeEditor/CustomType.h>
// #include <TypeEditor/Structure.h>
// #include <TypeEditor/x2cDataTree.h>
// #include <iFileIconProvider.h>

namespace MoonGlare::Tools::Editor::Modules {

//----------------------------------------------------------------------------------

EntityEditor::EntityEditor(QWidget *parent, SharedModuleManager modmgr)
    : iTypedChangeContainer(modmgr), shared_module_manager(modmgr) {

    ui = std::make_unique<Ui::EntityEditor>();
    ui->setupUi(this);
    ui->EntityTree->SetModuleManager(shared_module_manager);

    // SetSettingID("EntityEditor");
    // ui->EntityTree->SetSettingID("EntityTree");
    // SetChangesName("EntityEditor");
    // InsertChangesChild(ui->EntityTree, "Entities");

    connect(ui->EntityTree, &Widgets::EntityEditorWidget::Changed, [this]() { SetModifiedState(true); });
    connect(ui->action_save, &QAction::triggered, [this](auto) { SaveChanges(); });

    // auto ue = std::make_unique<BaseEntity>();
    // m_BaseEntity = ue.get();
    // ui->EntityTree->SetEntity(std::move(ue));
    file_system = shared_module_manager->QueryModule<iWritableFileSystem>();
}

EntityEditor::~EntityEditor() {
    ui.reset();
}

//----------------------------------------------------------------------------------
// iTabViewBase

std::string EntityEditor::GetTabTitle() const {
    return "EntityEditor";
}
bool EntityEditor::CanClose() const {
    return true;
}

//----------------------------------------------------------------------------------

//iEditor
bool EntityEditor::OpenData(const std::string &full_path, const iEditorInfo::FileHandleMethodInfo &method) {
    full_file_path = full_path;
    Reload();
    return true;
}

bool EntityEditor::Create(const std::string &full_path, const iEditorInfo::FileHandleMethodInfo &method) {
    full_file_path = full_path;
    Reload();
    return true;
}

//----------------------------------------------------------------------------------
//iChangeContainer

bool EntityEditor::SaveChanges() {
    if (!editable_entity) {
        return false;
    }
    auto entity_data = editable_entity->SerializeToJson();
    if (!file_system->WriteFileByPath(full_file_path, entity_data)) {
        AddLogf(Error, "File save failed");
        return false;
    }
    SetModifiedState(false);
    return true;
}

std::string EntityEditor::GetInfoLine() const {
    return "EntityEditor";
}

bool EntityEditor::DropChanges() {
    return false;
}

bool EntityEditor::DropChangesPossible() {
    return false;
}

QIcon EntityEditor::GetInfoIcon() const {
    return QIcon(ICON_16_ENTITYPATTERN_RESOURCE);
}

//----------------------------------------------------------------------------------
//iWidgetSettingsProvider
std::unordered_map<std::string, QWidget *> EntityEditor::GetStateSavableWidgets() const {
    return {
        // {"EntityTree", ui->EntityTree},
    };
}

//----------------------------------------------------------------------------------

void EntityEditor::Reload() {
    editable_entity = std::make_shared<EditableEntity>(shared_module_manager);

    std::string file_data;
    if (file_system->ReadFileByPath(full_file_path, file_data)) {
        editable_entity->DeserializeFromJson(file_data);
    } else {
        AddLogf(Warning, "Failed to read file %s", full_file_path.c_str());
    }

    ui->EntityTree->SetEntity(editable_entity);
}

//----------------------------------------------------------------------------------

#if 0

//----------------------------------------------------------------------------------

bool EntityEditor::Create(const std::string &LocationURI,
                                const QtShared::iEditorInfo::FileHandleMethodInfo &what) {
    QString qname;
    if (!QueryStringInput("Enter name:", qname, 2))
        return false;

    std::string name = qname.toLocal8Bit().constData();
    std::string URI = LocationURI + name + ".epx";

    auto fs = moduleManager->QueryModule<FileSystem>();
    if (!fs->CreateFile(URI)) {
        ErrorMessage("Failed during creating epx file");
        AddLog(Hint, "Failed to create epx: " << m_CurrentPatternFile);
        return false;
    }

    m_BaseEntity->GetName() = name;

    m_CurrentPatternFile = URI;
    SetModiffiedState(true);
    Refresh();

    AddLog(Hint, "Created epx file: " << URI);

    if (!SaveData()) {
        ErrorMessage("Failed during saving epx file");
        AddLog(Hint, "Failed to save epx: " << m_CurrentPatternFile);
    } else {
        SetModiffiedState(false);
    }

    return true;
}

bool EntityEditor::TryCloseData() {
    AddLog(Hint, "Trying to close epx: " << m_CurrentPatternFile);
    if (m_BaseEntity && IsChanged()) {
        if (!AskForPermission("There is a opened pattern. Do you want to close it?"))
            return false;
        if (AskForPermission("Save changes?"))
            SaveData();
    }
    m_CurrentPatternFile.clear();
    Clear();
    Refresh();
    return true;
}

bool EntityEditor::OpenData(const std::string &file) {
    if (!TryCloseData())
        return false;

    try {
        pugi::xml_document xdoc;

        auto fs = moduleManager->QueryModule<FileSystem>();
        StarVFS::ByteTable bt;
        if (!fs->GetFileData(file, bt)) {
            // todo: log sth
            throw false;
        }
        if (bt.byte_size() == 0) {
            throw true;
        }

        if (!xdoc.load_string((char *)bt.get())) {
            // TODO: log sth
            throw false;
        }

        if (!m_BaseEntity->Read(xdoc.document_element())) {
            AddLog(Hint, "Failed to read epx Entities: " << m_CurrentPatternFile);
            throw false;
        }

        m_CurrentPatternFile = file;
        AddLog(Hint, "Opened epx: " << m_CurrentPatternFile);
        Refresh();
        return true;
    } catch (...) {
        Clear();
        Refresh();
        ErrorMessage("Failure during opening data!");
        AddLog(Hint, "Failed to open epx: " << file);
        return false;
    }
}

bool EntityEditor::SaveData() {
    if (!IsChanged())
        return true;

    try {
        pugi::xml_document xdoc;
        if (!m_BaseEntity->Write(xdoc.append_child("Entity"))) {
            AddLog(Hint, "Failed to write epx Entities: " << m_CurrentPatternFile);
            throw false;
        }

        std::stringstream ss;
        xdoc.save(ss);
        StarVFS::ByteTable bt;
        bt.from_string(ss.str());

        auto fs = moduleManager->QueryModule<FileSystem>();
        if (!fs->SetFileData(m_CurrentPatternFile.c_str(), bt)) {
            // todo: log sth
            return false;
        }

        SetModiffiedState(false);
        AddLog(Hint, "Saved epx: " << m_CurrentPatternFile);

        return true;
    } catch (...) {
        AddLog(Hint, "Failed to save epx: " << m_CurrentPatternFile);
        return false;
    }
}

//----------------------------------------------------------------------------------

#endif

} // namespace MoonGlare::Tools::Editor::Modules
