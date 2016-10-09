#ifndef QuickActions_H
#define QuickActions_H

#include <DockWindow.h>

namespace Ui { class QuickActions; }

class QuickActions : public QtShared::DockWindow {
	Q_OBJECT
public:
	QuickActions(QWidget *parent = 0);
	~QuickActions();

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;
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
