#ifndef MEMORYSTATEWIDGET_H
#define MEMORYSTATEWIDGET_H

#include "ResourceEditorBaseTab.h"

namespace Ui { class MemoryStateWidget; };

class MemoryStateWidget : public ResourceEditorBaseTab {
	Q_OBJECT
public:
	MemoryStateWidget(QWidget *parent = 0);
	~MemoryStateWidget();
protected:
	class MemoryRequest; 
	void ResetTreeView();
private:
	Ui::MemoryStateWidget *ui;
	std::unique_ptr<QStandardItemModel> m_ViewModel;
public slots:
	void Refresh() override;
protected slots:
;
};

#endif // LUAGLOBALSTATEWIDGET_H
