#ifndef DOCKWINDOW_H
#define DOCKWINDOW_H

#include "mgdtSettings.h"

class DockWindow 
	: public QDockWidget
	, public iSettingsUser
	, public std::enable_shared_from_this<DockWindow> {
	Q_OBJECT
public:
	DockWindow(QWidget *parent, bool AutoRefresh = false);
	~DockWindow();

	void closeEvent(QCloseEvent * event);
	void showEvent(QShowEvent * event);
	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;

	void SetAutoRefresh(bool value, unsigned Interval = 1000);
public slots:
	virtual void Refresh();
signals:
	void WindowClosed(DockWindow* Sender);
protected slots:
	void OnEngineDisconnected();
	void OnEngineConnected();
	void OnRefreshTimer();
private:
	bool m_AutoRefresh;
	std::unique_ptr<QTimer> m_RefreshTimer;
};

#endif // SUBWINDOW_H
