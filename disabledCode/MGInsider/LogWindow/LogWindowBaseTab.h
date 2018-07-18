#pragma once
#include "RemoteConsole.h"

class LogWindowBaseTab : public QWidget, public RemoteConsoleRequestQueue {
	Q_OBJECT
public:
	LogWindowBaseTab(QWidget *parent);
	~LogWindowBaseTab();

	const QString& GetName() const { return m_Name; }

	static Space::RTTI::TypeInfo* GetStaticTypeInfo() { return nullptr; }
protected:
	void SetName(QString Name) { m_Name.swap(Name); }
private:
	QString m_Name;

signals:
public slots:
	virtual void Refresh() { }
};

using LogWindowTabRegister = Space::DynamicClassRegister<LogWindowBaseTab, QWidget*>;
