#pragma once

#include <QWidget>
#include <change_container.hpp>
#include <editable_entity.hpp>
#include <editor_provider.h>
#include <issue_reporter.hpp>
#include <main_window_tabs.hpp>
#include <memory>
#include <runtime_modules.h>
#include <runtime_modules/widget_settings_provider.hpp>
#include <user_questions.hpp>
#include <writable_file_system.h>

// #include "../Notifications.h"
// #include <DataModels/EntityEditorModel.h>
// #include <DockWindow.h>
// #include <ToolBase/Modules/ChangesManager.h>
// #include <iEditor.h>

namespace Ui {
class EntityEditor;
}

namespace MoonGlare::Tools::Editor::Modules {

// class EditableEntity;
// class EditableComponent;
// class EditablePattern;

// struct EditableComponentValueInfo {
//     EditableComponent *m_OwnerComponent = nullptr;
//     TypeEditor::StructureValue *m_ValueInterface = nullptr;
//     EditableEntity *m_EditableEntity = nullptr;
//     QStandardItem *m_Item = nullptr;

//     operator bool() const { return m_Item && m_OwnerComponent; }
// };

// struct EditableItemInfo {
//     QStandardItem *m_Item = nullptr;
//     EditableEntity *m_EditableEntity = nullptr;
//     EditableEntity *m_Parent = nullptr;
//     bool m_PatternURIMode = false;

//     operator bool() const { return m_Item && m_EditableEntity; }
// };

// struct UserRoles {
//     enum {
//         // EditableItemInfo = Qt::UserRole + 1,
//         // EditableComponentValueInfo,
//     };
// };

class EntityEditor : public QWidget,
                     public iEditor,
                     public iTypedChangeContainer<EntityEditor>,
                     public iTabViewBase,
                     public UserQuestions,
                     public RuntineModules::iWidgetSettingsProvider,
                     public std::enable_shared_from_this<EntityEditor> {
    // Q_OBJECT;

public:
    EntityEditor(QWidget *parent, SharedModuleManager modmgr);
    virtual ~EntityEditor();
    // virtual bool DoSaveSettings(pugi::xml_node node) const override;
    // virtual bool DoLoadSettings(const pugi::xml_node node) override;

    // virtual bool SaveChanges() override { return SaveData(); }
    // virtual std::string GetInfoLine() const override { return m_CurrentPatternFile; }

    // iTabViewBase
    std::string GetTabTitle() const override;
    bool CanClose() const override;

    //iEditor
    bool OpenData(const std::string &full_path, const iEditorInfo::FileHandleMethodInfo &method) override;
    bool Create(const std::string &full_path, const iEditorInfo::FileHandleMethodInfo &method) override;

    //iChangeContainer
    bool SaveChanges() override;
    std::string GetInfoLine() const override;
    bool DropChanges() override;
    bool DropChangesPossible() override;
    QIcon GetInfoIcon() const override;

    //iWidgetSettingsProvider
    std::unordered_map<std::string, QWidget *> GetStateSavableWidgets() const override;

    // virtual bool Create(const std::string &LocationURI,
    //                     const QtShared::iEditorInfo::FileHandleMethodInfo &what) override;
    // virtual bool OpenData(const std::string &file) override;
    // virtual bool SaveData() override;
    // virtual bool TryCloseData() override;

    // void Clear();

private:
    std::unique_ptr<Ui::EntityEditor> ui;
    std::string full_file_path;
    SharedModuleManager shared_module_manager;
    std::shared_ptr<EditableEntity> editable_entity;
    std::shared_ptr<iWritableFileSystem> file_system;

    void Reload();
};

} // namespace MoonGlare::Tools::Editor::Modules

// Q_DECLARE_METATYPE(MoonGlare::Editor::EntityEditor::EditableItemInfo);
// Q_DECLARE_METATYPE(MoonGlare::Editor::EntityEditor::EditableComponentValueInfo);
