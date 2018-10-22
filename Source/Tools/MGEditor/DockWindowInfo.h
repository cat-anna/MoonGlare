#ifndef BaseDockWindowModule_H
#define BaseDockWindowModule_H

#include <DockWindow.h>
#include <ToolBase/Module.h>

namespace MoonGlare {
namespace QtShared {

class BaseDockWindowModule 
		: public QObject
		, public iModule
		, public iSettingsUser {
	Q_OBJECT;
public:
	BaseDockWindowModule(SharedModuleManager modmgr);
	virtual ~BaseDockWindowModule();

	const QString& GetIconResName() const{ return m_IconResName; }
	const QString& GetDisplayName() const { return m_DisplayName; }
	const QString& GetShortcut() const { return m_ShortCut; }
	bool IsMainMenu() const { return m_DisableMainMenu; }

	QIcon GetIcon() const { return QIcon(GetIconResName()); }
	QKeySequence GetKeySequence() const { return QKeySequence(GetShortcut()); }

	std::shared_ptr<DockWindow> GetInstance(QWidget *parent = nullptr);
	void ReleaseInstance();
     
    bool Finalize() override;
public slots:
	void Show();
protected:
	void SetIconResName(QString v) { m_IconResName.swap(v); }
	void SetDisplayName(QString v) { m_DisplayName.swap(v); }
	void SetShortcut(QString v) { m_ShortCut.swap(v); }
	void SetMainMenu(bool v) { m_DisableMainMenu = v; }

	virtual std::shared_ptr<DockWindow> CreateInstance(QWidget *parent) = 0;

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;
protected slots:
	void WindowClosed(DockWindow* Sender);
private:
	QString m_IconResName;
	QString m_DisplayName;
	QString m_ShortCut;
	std::shared_ptr<DockWindow> m_Instance;
	bool m_DisableMainMenu;
};

template<typename DOCK>
class DockWindowModule : public BaseDockWindowModule {
public:
	DockWindowModule(SharedModuleManager modmgr) : BaseDockWindowModule(std::move(modmgr)) { }

protected:
    virtual std::shared_ptr<DockWindow> CreateInstance(QWidget *parent) {
		return std::make_shared<DOCK>(parent, WeakModule(shared_from_this()));
	}
};


using SharedBaseDockWindowModule = std::shared_ptr<BaseDockWindowModule>;

} //namespace QtShared
} //namespace MoonGlare

#endif // SUBWINDOW_H
