#ifndef OrbitLoggerStatus_H
#define OrbitLoggerStatus_H

#include <DockWindow.h>
#include "RemoteConsole.h"

namespace Ui { class OrbitLoggerStatus; }

class OrbitLoggerStatus : public DockWindow, public RemoteConsoleRequestQueue {
	Q_OBJECT
public:
	OrbitLoggerStatus(QWidget *parent = 0);
	~OrbitLoggerStatus();

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;

	void ShowData(const InsiderApi::PayLoad_OrbitLoggerStateResponse &data);
protected slots:
	void RefreshView();
private:
	std::unique_ptr<Ui::OrbitLoggerStatus> m_Ui;
	std::unique_ptr<QStandardItemModel> m_ViewModel;
};

#endif // MAINFORM_H
