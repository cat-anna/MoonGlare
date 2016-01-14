#pragma once
#include "LogWindowBaseTab.h"

namespace Ui { class InsiderLogWidget; };

class InsiderLogWidget : public LogWindowBaseTab {
	Q_OBJECT
public:
	InsiderLogWidget(QWidget *parent = 0);
	~InsiderLogWidget();
protected:
	void ResetTreeView();
private:
	Ui::InsiderLogWidget *ui;
	std::unique_ptr<QStandardItemModel> m_ViewModel;
public slots:
protected slots:
};
