#pragma once
#include "LogWindowBaseTab.h"

namespace Ui { class EngineLogWidget; };

class EngineLogWidget : public LogWindowBaseTab {
	Q_OBJECT
public:
	EngineLogWidget(QWidget *parent = 0);
	~EngineLogWidget();
protected:
private:
	Ui::EngineLogWidget *ui;
public slots:
protected slots:
};
