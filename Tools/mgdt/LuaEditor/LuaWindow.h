#ifndef LUAWINDOW_H
#define LUAWINDOW_H

#include "RemoteConsole.h"
#include "ui_LuaWindow.h"
#include "EditorTab.h"

class LuaCompiler;

class LuaWindow : public QMainWindow
{
	Q_OBJECT;
public:
	LuaWindow(QWidget *parent = 0);
	~LuaWindow();

	void closeEvent(QCloseEvent * event);

	void RefreshOpenedFiles();
	void RefreshEditorStatus(EditorTab *editor = nullptr);
private:
	Ui::LuaWindow ui;
	EditorTab* CurrentEditor();
	std::unique_ptr<QStandardItemModel> m_OpenedFilesViewModel;
	std::unique_ptr<QStandardItemModel> m_StatusViewModel;
	
	void ResetModelViews();
	void ResetStatusModelView();
	void CreateEditor(const QString &path, EditorFileSource mode);
public slots:
	void OpenEngineScirpt(const QString &path);
	void OpenScriptFile(const QString &file);
	void ValidateScript(EditorTab *sender);
protected slots:
	void NewScriptAction();
	void OpenScriptAction();
	void SaveScriptAction();
	void CloseScriptAction();
	void CloseAllScriptsAction();
	void SendScriptAction();
	void SendSelectedScriptAction();
	void ReloadAction();
	void ReloadAllAction();
	void KillScirptsAction();
	void ScriptTabStateChanged(EditorTab *);
	void TabContextMenuRequested(EditorTab *sender, QPoint pos);
	void OpenedFilesItemClicked(QModelIndex index);
	void EditorStatusItemClicked(QModelIndex index);
	void ValidateScriptAction();
signals:
	void WindowClosed();
};

#define FileFilter_Scripts "Lua scripts(*.lua)"

#endif // LUAWINDOW_H
