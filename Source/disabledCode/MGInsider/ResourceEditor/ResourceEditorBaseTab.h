#ifndef RESOURCEEDITORBASETAB_H
#define RESOURCEEDITORBASETAB_H

#include "RemoteConsole.h"

class ResourceEditorBaseTab : public QWidget, public RemoteConsoleRequestQueue {
	Q_OBJECT
public:
	ResourceEditorBaseTab(QWidget *parent);
	~ResourceEditorBaseTab();

	void showEvent(QShowEvent * event);

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

using ResourceEditorTabRegister = Space::DynamicClassRegister<ResourceEditorBaseTab, QWidget*>;

#endif // RESOURCEEDITORBASETAB_H
