#pragma once
#include <SubWindow.h>

namespace Ui {class LogWindow;};
class LogWindowBaseTab;

class LogWindow : public SubWindow {
	Q_OBJECT
public:
	LogWindow(QWidget *parent = 0);
	~LogWindow();

public slots:
protected:
	LogWindowBaseTab* CurrentEditor();
private:
	Ui::LogWindow *ui;
};

