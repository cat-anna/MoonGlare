#ifndef DOCKWINDOW_H
#define DOCKWINDOW_H

#include "mgdtSettings.h"

class DockWindow : public QDockWidget, public iSettingsUser {
	Q_OBJECT
public:
	DockWindow(QWidget *parent);
	~DockWindow();

	void closeEvent(QCloseEvent * event);
	void showEvent(QShowEvent * event);
	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;
signals:
	void WindowClosed(DockWindow* Sender);
protected:
private:
};

#endif // SUBWINDOW_H
