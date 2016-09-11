#ifndef DOCKWINDOW_H
#define DOCKWINDOW_H

#include <qobject.h>
#include "iSettingsUser.h"
#include "qtUtils.h"

namespace MoonGlare {
namespace QtShared {

class DockWindow 
	: public QDockWidget
	, public UserQuestions
	, public iSettingsUser
	, public std::enable_shared_from_this<DockWindow> {
	Q_OBJECT;
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

} //namespace QtShared
} //namespace MoonGlare

#endif // SUBWINDOW_H
