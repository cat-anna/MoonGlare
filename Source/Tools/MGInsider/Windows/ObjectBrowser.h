#ifndef QuickActions_H
#define QuickActions_H

#include <DockWindow.h>

namespace Ui { class ObjectBrowser; }

class ObjectBrowser : public DockWindow {
	Q_OBJECT
public:
	ObjectBrowser(QWidget *parent = 0);
	~ObjectBrowser();

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;
protected:
private:
	std::unique_ptr<Ui::ObjectBrowser> m_Ui;
	std::unique_ptr<QStandardItemModel> m_ViewModel;
public slots:
	void NewAction();
	void RemoveAction();
	void ModifyAction();
protected slots:
};

#endif // MAINFORM_H
