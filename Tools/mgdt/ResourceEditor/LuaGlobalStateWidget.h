#ifndef LUAGLOBALSTATEWIDGET_H
#define LUAGLOBALSTATEWIDGET_H

#include "ResourceEditorBaseTab.h"
namespace Ui { class LuaGlobalStateWidget; };

class LuaGlobalStateWidget : public ResourceEditorBaseTab {
	Q_OBJECT
public:
	LuaGlobalStateWidget(QWidget *parent = 0);
	~LuaGlobalStateWidget();
protected:
	class LuaRequest;

	void ResetTreeView();
public slots:
	void Refresh() override;
private:
	Ui::LuaGlobalStateWidget *ui;
	std::unique_ptr<QStandardItemModel> m_ViewModel;
};

#endif // LUAGLOBALSTATEWIDGET_H
