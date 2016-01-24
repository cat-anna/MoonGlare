#ifndef QuickActions_H
#define QuickActions_H

#include <SubWindow.h>

class SubWindow;
class LuaWindow;
class ResourceBrowser;

namespace Ui { class QuickActions; }

class QuickActions : public SubWindow {
	Q_OBJECT
public:
	QuickActions(QWidget *parent = 0);
	~QuickActions();
protected:
private:
	Ui::QuickActions *ui;
	std::unique_ptr<QStandardItemModel> m_ViewModel;
	public slots:
	void NewAction();
	void RemoveAction();
	void ModifyAction();
	void ExecuteAction();
	void RefreshView();
protected slots:
	void ActionDoubleClicked(const QModelIndex& index);
};

#endif // MAINFORM_H
