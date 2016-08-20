#ifndef MAINFORM_H
#define MAINFORM_H

#include "RemoteConsole.h"
#include "DockWindowInfo.h"

class SubWindow;
class LuaWindow;
class ResourceBrowser;
namespace Ui { class MainForm; }

enum class EngineStateValue {
	None,
	EngineVersion,
	EngineBuildDate,
	EngineExeName,
	EngineState,
	EnginePing,

	RemoteConsoleMessagesSent,
	RemoteConsoleMessagesTimedout,
	RemoteConsoleMessagesRecived,

	CurrentScene,
};

struct EngineStateValueCaptions : Space::EnumConverter < EngineStateValue, EngineStateValue::None > {
	EngineStateValueCaptions() {
		Add("None", Enum::None);
		Add("Version", Enum::EngineVersion);
		Add("Build date", Enum::EngineBuildDate);
		Add("Exename", Enum::EngineExeName);
		Add("Ping time", Enum::EnginePing);
		Add("Engine state", Enum::EngineState);
		Add("Current scene", Enum::CurrentScene);
		Add("Messages sent", Enum::RemoteConsoleMessagesSent);
		Add("Messages recived", Enum::RemoteConsoleMessagesRecived);
		Add("Messages timedout", Enum::RemoteConsoleMessagesTimedout);
	}
};
using EngineStateValueCaptionValues = Space::EnumConverterHolder < EngineStateValueCaptions >;

struct EngineStateValueGroups : Space::EnumConverter < EngineStateValue, EngineStateValue::None > {
	EngineStateValueGroups() {
		Add("Engine", Enum::EngineVersion);
		Add("Engine", Enum::EngineBuildDate);
		Add("Engine", Enum::EngineExeName);
		Add("Engine", Enum::EnginePing);
		Add("", Enum::EngineState);
		Add("", Enum::CurrentScene);
		Add("", Enum::None);
	}
};
using EngineStateValueGroupsValues = Space::EnumConverterHolder < EngineStateValueGroups >;

class MainForm : public QMainWindow {
	Q_OBJECT
public:
	MainForm(QWidget *parent = 0);
	~MainForm();

	static MainForm* Get();

protected:
	void closeEvent(QCloseEvent * event);
	void showEvent(QShowEvent * event);
private:
	Ui::MainForm *ui;
	std::vector<SharedDockWindowInfo> m_DockWindows;

	std::unique_ptr<LuaWindow> m_LuaEditor;
	std::unique_ptr<SubWindow> m_ResourceBrowser;
	std::unique_ptr<SubWindow> m_LogWindow;
	
	std::unique_ptr<QStandardItemModel> m_StateModelView;
	std::map<EngineStateValue, QStandardItem*> m_StateValues;
	std::unordered_map<std::string, QStandardItem*> m_StateGroupValues;
public slots:
	void OpenEngineScript(const QString &file);
	void EngineStateValueChanged(EngineStateValue type, const QString &Value, const QString& icon = "");
protected slots:
	void RemoteConsoleStateChanged(RemoteConsoleState state);
	void ShowScriptEditor();
	void ScriptEditorClosed();
	void ShowResourceBrowser();
	void ResourceBrowserClosed();
	void ShowLogWindow();
	void LogWindowClosed();
};

#endif // MAINFORM_H
