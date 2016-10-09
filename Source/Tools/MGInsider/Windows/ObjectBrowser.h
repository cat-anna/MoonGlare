#ifndef QuickActions_H
#define QuickActions_H

#include <DockWindow.h>
#include "RemoteConsoleObserver.h"

namespace Ui { class ObjectBrowser; }

class ObjectBrowser  
		: public QtShared::DockWindow
		, protected RemoteConsoleRequestQueue {
	Q_OBJECT
public:
	ObjectBrowser(QWidget *parent = 0);
	~ObjectBrowser();

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;

	QStandardItemModel* GetModel() { return m_ViewModel.get(); }
protected:
	class Request;
private:
	std::unique_ptr<Ui::ObjectBrowser> m_Ui;
	std::unique_ptr<QStandardItemModel> m_ViewModel;
	std::shared_ptr<Request> m_Request;
public slots:
	void Refresh() override;
protected slots:
	void ItemClicked(const QModelIndex&);
};

#endif // MAINFORM_H
