#ifndef QuickActions_H
#define QuickActions_H

#include <DockWindow.h>

namespace Ui { class ObjectBrowser; }

class ObjectBrowser : public DockWindow, protected RemoteConsoleRequestQueue {
	Q_OBJECT
public:
	ObjectBrowser(QWidget *parent = 0);
	~ObjectBrowser();

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;

	struct ObjectData;
	ObjectData* GetData() { return m_Data.get(); }
	QStandardItemModel* GetModel() { return m_ViewModel.get(); }
protected:
	class Request;
private:
	std::unique_ptr<Ui::ObjectBrowser> m_Ui;
	std::unique_ptr<ObjectData> m_Data;
	std::unique_ptr<QStandardItemModel> m_ViewModel;
	std::unique_ptr<QStandardItemModel> m_DetailsModel;
	MoonGlare::Handle m_SelectedItem;
public slots:
	void RefreshView();
	void RefreshDetailsView();
	protected slots:
	void ItemClicked(const QModelIndex&);
};

#endif // MAINFORM_H
