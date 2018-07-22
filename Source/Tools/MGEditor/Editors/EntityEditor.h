/*
  * Generated by cppsrc.sh
  * On 2016-07-22  0:08:40,44
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef FileSystemViewer_H
#define FileSystemViewer_H

#include <DockWindow.h>
#include <iEditor.h>
#include <ChangesManager.h>
#include <DataModels/EntityEditorModel.h>

#include "../Notifications.h"

namespace Ui { class EntityEditor; }

namespace MoonGlare {
namespace Editor {
namespace EntityEditor {

class EditableEntity;
class EditableComponent;
class EditablePattern;

struct EditableComponentValueInfo {
	EditableComponent *m_OwnerComponent = nullptr;
	TypeEditor::StructureValue *m_ValueInterface = nullptr;
	EditableEntity *m_EditableEntity = nullptr;
	QStandardItem *m_Item = nullptr;

	operator bool() const { return m_Item && m_OwnerComponent; }
};

struct EditableItemInfo {
	QStandardItem *m_Item = nullptr;
	EditableEntity *m_EditableEntity = nullptr;
	EditableEntity *m_Parent = nullptr;
	bool m_PatternURIMode = false;

	operator bool() const { return m_Item && m_EditableEntity; }
};

struct UserRoles {
	enum {
		EditableItemInfo = Qt::UserRole + 1,
		EditableComponentValueInfo,
	};
};

class EntityEditorWindow
		: public QtShared::DockWindow 
		, public QtShared::iChangeContainer
		, public QtShared::iEditor {
	Q_OBJECT;
public:
	EntityEditorWindow(QWidget *parent, QtShared::SharedModuleManager modmgr);
 	virtual ~EntityEditorWindow();
	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;

	virtual bool SaveChanges() override { return SaveData(); }
	virtual bool CanDropChanges() const override  { return false; }
	virtual bool DropChanges() override { return false; }
	virtual std::string GetInfoLine() const override { return m_CurrentPatternFile; }


	class BaseEntity : public QtShared::DataModels::EditableEntity {};
public slots:
	virtual bool Create(const std::string &LocationURI, const QtShared::iEditorInfo::FileHandleMethodInfo& what) override;
	virtual bool OpenData(const std::string &file) override;
	virtual bool SaveData() override;
	virtual bool TryCloseData() override;

	void Refresh();
	void Clear();
protected slots:
	void ProjectChanged(Module::SharedDataModule datamod);
private: 
	std::unique_ptr<Ui::EntityEditor> m_Ui;
	BaseEntity *m_BaseEntity;
	std::string m_CurrentPatternFile;
    QtShared::SharedModuleManager moduleManager;
};

} //namespace EntityEditor 
} //namespace Editor 
} //namespace MoonGlare 

Q_DECLARE_METATYPE(MoonGlare::Editor::EntityEditor::EditableItemInfo);
Q_DECLARE_METATYPE(MoonGlare::Editor::EntityEditor::EditableComponentValueInfo);

#endif
