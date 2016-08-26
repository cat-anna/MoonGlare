#ifndef MemoryState_H
#define MemoryState_H

#include <DockWindow.h>
#include "RemoteConsole.h"

namespace Ui { class MemoryState; };

class MemoryState 
		: public DockWindow
		, public RemoteConsoleRequestQueue {
	Q_OBJECT
public:
	MemoryState(QWidget *parent = 0);
	~MemoryState();

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;
protected:
	class MemoryRequest; 
	void ResetTreeView();
private:
	std::unique_ptr<Ui::MemoryState> m_Ui;
	std::unique_ptr<QStandardItemModel> m_ViewModel;
public slots:
	void Refresh() override;
protected slots:
};

#endif // LUAGLOBALSTATEWIDGET_H