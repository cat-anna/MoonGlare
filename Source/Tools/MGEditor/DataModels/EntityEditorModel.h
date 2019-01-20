/*
  * Generated by cppsrc.sh
  * On 2016-07-22  0:08:40,44
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef EntityEditorModel_H
#define EntityEditorModel_H

#include <ToolBase/Modules/ChangesManager.h>
#include <ToolBase/UserQuestions.h>
#include <ToolBase/iSettingsUser.h>
#include <iEditor.h>

#include <TypeEditor/ComponentInfo.h>
#include <TypeEditor/CustomType.h>
#include <TypeEditor/Structure.h>

#include <Source/Engine/Core/Component/nfComponent.h>

#include "EditableEntity.h"

namespace Ui { class EntityEditorModel; }

namespace MoonGlare {
namespace QtShared {
namespace DataModels {

class EntityEditorModel
		: public QWidget 
		, public UserQuestions
		, public iChangeContainer
		, public iSettingsUser
{
	Q_OBJECT;
public:
	EntityEditorModel(QWidget *parent);
 	virtual ~EntityEditorModel();

	struct UserRoles {
		enum {
			EditableItemInfo = Qt::UserRole + 1,
			EditableComponentValueInfo,
		};
	};

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

	void SetEntity(std::unique_ptr<EditableEntity> NewRoot) {
		m_RootEntity.swap(NewRoot);
		Refresh();
	}

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;

    void SetModuleManager(SharedModuleManager mm);

//iChangeContainer
    bool SaveChanges() override { SetModiffiedState(false); return true; };
public slots:
	void Refresh();
	void RefreshDetails();
protected slots:
	void EntityClicked(const QModelIndex& index);
	void ComponentClicked(const QModelIndex& index);
	void ComponentChanged(QStandardItem *item);
	void EntityChanged(QStandardItem *item);
	void ComponentContextMenu(const QPoint &);
	void EntityContextMenu(const QPoint &);

	void CutEntity();
	void CopyEntity();
	void PasteEntity();
	
	void ShowAddComponentMenu();
	//void ProjectChanged(Module::SharedDataModule datamod);
private: 
	std::unique_ptr<Ui::EntityEditorModel> m_Ui;
	std::unique_ptr<QStandardItemModel> m_EntityModel;
	std::unique_ptr<QStandardItemModel> m_ComponentModel;
	std::unique_ptr<EditableEntity> m_RootEntity;
	std::unique_ptr<QMenu> m_AddComponentMenu;
	//std::string m_CurrentPatternFile;
	EditableItemInfo m_CurrentItem;
	EditableComponentValueInfo m_CurrentComponent;
    SharedModuleManager moduleManager;
};

} //namespace DataModels
} //namespace QtShared
} //namespace MoonGlare

Q_DECLARE_METATYPE(MoonGlare::QtShared::DataModels::EntityEditorModel::EditableItemInfo);
Q_DECLARE_METATYPE(MoonGlare::QtShared::DataModels::EntityEditorModel::EditableComponentValueInfo);

#endif
