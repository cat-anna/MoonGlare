#ifndef Notifications_H
#define Notifications_H

namespace MoonGlare {
namespace Editor {

namespace Module {
	class DataModule;
	using SharedDataModule = std::shared_ptr<DataModule>;
}

class Notifications : public QObject {
	Q_OBJECT;
public:
	virtual ~Notifications();
	static Notifications* Get();

	static void SendSetStateValue(const std::string& ID, const std::string &Value, const std::string& Icon = "") { Get()->SetStateValue(ID, Value, Icon); }
	static void SendRemoveStateValue(const std::string& ID) { Get()->RemoveStateValue(ID); }

	static void SendRefreshView() { Get()->RefreshView(); }
	static void SendSettingsChanged() { Get()->SettingsChanged(); }
	static void SendModuleConfigurationChanged() { Get()->ModuleConfigurationChanged(); }

	static void SendProjectChanged(Module::SharedDataModule datamod) { Get()->ProjectChanged(std::move(datamod)); }
signals:
	void SetStateValue(const std::string& ID, const std::string &Value, const std::string& Icon = "");
	void RemoveStateValue(const std::string& ID);

	void RefreshView();
	void SettingsChanged();
	void ModuleConfigurationChanged();
	void ProjectChanged(Module::SharedDataModule datamod);
protected:
	Notifications();
};

} //namespace Editor
} //namespace MoonGlare

#endif // SUBWINDOW_H
