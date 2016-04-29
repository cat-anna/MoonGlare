#ifndef QuickActions_H
#define QuickActions_H

#include <DockWindow.h>
#include "RemoteConsole.h"

namespace Ui { class EngineStatus; }

class EngineStatus : public DockWindow, public RemoteConsoleRequestQueue {
	Q_OBJECT
public:
	EngineStatus(QWidget *parent = 0);
	~EngineStatus();

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;

public slots:
	void SetValue(const std::string& ID, const std::string &Value, const std::string& Icon = "");
	void RemoveValue(const std::string& ID);
protected:
private:
	std::unique_ptr<Ui::EngineStatus> m_Ui;
	std::unique_ptr<QStandardItemModel> m_ViewModel;
	std::unordered_map<std::string, QStandardItem*> m_Values;
};

#endif // MAINFORM_H
