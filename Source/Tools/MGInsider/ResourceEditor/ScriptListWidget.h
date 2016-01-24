#ifndef SCRIPTLISTWIDGETT_H
#define SCRIPTLISTWIDGETT_H

#include "ResourceEditorBaseTab.h"

namespace Ui { class ScriptListWidget; };

class ScriptListWidget : public ResourceEditorBaseTab {
	Q_OBJECT
public:
	ScriptListWidget(QWidget *parent = 0);
	~ScriptListWidget();
protected:
	class LuaRequest; 
	void ResetTreeView();
private:
	QAction *m_EditScriptAction;
	Ui::ScriptListWidget *ui;
	std::unique_ptr<QStandardItemModel> m_ViewModel;
public slots:
	void Refresh() override;
protected slots:
	void EditScriptAction();
	void ScriptDoubleClicked(const QModelIndex& index);
;
};

#endif // LUAGLOBALSTATEWIDGET_H
