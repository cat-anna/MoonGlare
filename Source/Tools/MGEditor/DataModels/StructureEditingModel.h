#pragma once
#ifndef StructureEditingModel_H
#define StructureEditingModel_H

#include <QtWidgets/QTreeView>

#include <TypeEditor/CustomType.h>
#include <TypeEditor/Structure.h>

#include <ToolBase/Module.h>
#include <ToolBase/Modules/ChangesManager.h>
#include <ToolBase/Modules/iSettingsUser.h>
#include <ToolBase/UserQuestions.h>

#include "EditableEntity.h"

namespace MoonGlare {
namespace QtShared {
namespace DataModels {

class StructureEditingModel 
		: public QTreeView
		, public MoonGlare::Module::iSettingsUser
		, public MoonGlare::Module::iChangeContainer {
	Q_OBJECT;
public:
	StructureEditingModel(QWidget *parent);
	~StructureEditingModel();

	void CreateStructure(TypeEditor::SharedStructureInfo structure);
	void SetStructure(TypeEditor::UniqueStructure&& structure);

	TypeEditor::Structure* GetStructure() { return m_Structure.get(); }

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;

    void SetModuleManager(SharedModuleManager mm);

//iChangeContainer
    bool SaveChanges() override { SetModiffiedState(false); return true; }
public slots:
	void Refresh();
protected slots:
	void ItemChanged(QStandardItem *item);
private:
	std::unique_ptr<QStandardItemModel> m_Model;
	TypeEditor::UniqueStructure m_Structure;
    SharedModuleManager moduleManager;
};

} //namespace DataModels
} //namespace QtShared
} //namespace MoonGlare

#endif // StructureEditingModel_H
