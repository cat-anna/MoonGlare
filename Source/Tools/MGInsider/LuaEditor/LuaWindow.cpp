#include PCH_HEADER
#include "mgdtSettings.h"
#include "LuaWindow.h"

LuaWindow::LuaWindow(QWidget *parent):
		QMainWindow(parent) {
	ui.setupUi(this);
	connect(ui.actionOpenScript, SIGNAL(triggered()), SLOT(OpenScriptAction()));
	connect(ui.actionSaveScript, SIGNAL(triggered()), SLOT(SaveScriptAction()));
	connect(ui.actionCloseScript, SIGNAL(triggered()), SLOT(CloseScriptAction()));
	connect(ui.actionClose_all_scipts, SIGNAL(triggered()), SLOT(CloseAllScriptsAction()));
	connect(ui.actionSendScript, SIGNAL(triggered()), SLOT(SendScriptAction()));
	connect(ui.actionReload_script, SIGNAL(triggered()), SLOT(ReloadAction()));
	connect(ui.actionReload_all_scripts, SIGNAL(triggered()), SLOT(ReloadAllAction()));
	connect(ui.actionNew_script, SIGNAL(triggered()), SLOT(NewScriptAction()));
	connect(ui.actionKill_script_instances, SIGNAL(triggered()), SLOT(KillScirptsAction()));
	connect(ui.actionValidate_script, SIGNAL(triggered()), SLOT(ValidateScriptAction()));

	connect(ui.OpenedFilesList, SIGNAL(clicked(const QModelIndex)), this, SLOT(OpenedFilesItemClicked(QModelIndex)));
	connect(ui.StatusTreeView, SIGNAL(clicked(const QModelIndex)), this, SLOT(EditorStatusItemClicked(QModelIndex)));

	ui.StatusTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.OpenedFilesList->setEditTriggers(QAbstractItemView::NoEditTriggers);

	if (mgdtSettings::get().Recent.ScriptFiles.empty()) {
		OpenScriptFile("");
	} else {
		for (auto &it : mgdtSettings::get().Recent.ScriptFiles) {
			switch (it.Source) {
			case EditorFileSource::RawFile:
				OpenScriptFile(QString(it.Location.c_str()));
				break;
			case EditorFileSource::Engine:
				OpenEngineScirpt(QString(it.Location.c_str()));
				break;
			}
		}
	}
	RefreshOpenedFiles();

	GetSettings().Window.LuaEditor.Apply(this);
}

LuaWindow::~LuaWindow() {
	GetSettings().Window.LuaEditor.Store(this);
	ResetModelViews();
}

void LuaWindow::ResetModelViews() {
	auto &settings = mgdtSettings::get();
	if (m_OpenedFilesViewModel) {
		settings.Editor.OpenedFiles.NameCollumnSize = ui.OpenedFilesList->columnWidth(2);
		settings.Editor.OpenedFiles.TypeCollumnSize = ui.OpenedFilesList->columnWidth(3);
	}
	ui.OpenedFilesList->setModel(nullptr);
	m_OpenedFilesViewModel = std::make_unique<QStandardItemModel>();

	m_OpenedFilesViewModel->setHorizontalHeaderItem(0, new QStandardItem(""));
	m_OpenedFilesViewModel->setHorizontalHeaderItem(1, new QStandardItem(""));
	m_OpenedFilesViewModel->setHorizontalHeaderItem(2, new QStandardItem("Name"));
	m_OpenedFilesViewModel->setHorizontalHeaderItem(3, new QStandardItem("Type"));

	ui.OpenedFilesList->setModel(m_OpenedFilesViewModel.get());

	ui.OpenedFilesList->setColumnWidth(0, 20);
	ui.OpenedFilesList->setColumnWidth(1, 20);
	ui.OpenedFilesList->setColumnWidth(2, settings.Editor.OpenedFiles.NameCollumnSize);
	ui.OpenedFilesList->setColumnWidth(3, settings.Editor.OpenedFiles.TypeCollumnSize);
}

void LuaWindow::ResetStatusModelView() {
	//auto &settings = mgdtSettings::get();
	//StatusTreeView
	if (m_StatusViewModel) {
		//settings.Editor.OpenedFiles.NameCollumnSize = ui.OpenedFilesList->columnWidth(2);
		//settings.Editor.OpenedFiles.TypeCollumnSize = ui.OpenedFilesList->columnWidth(3);
	}
	ui.StatusTreeView->setModel(nullptr);
	m_StatusViewModel = std::make_unique<QStandardItemModel>();

	m_StatusViewModel->setHorizontalHeaderItem(0, new QStandardItem("Type"));
	m_StatusViewModel->setHorizontalHeaderItem(1, new QStandardItem("File"));
	m_StatusViewModel->setHorizontalHeaderItem(2, new QStandardItem("Line"));
	m_StatusViewModel->setHorizontalHeaderItem(3, new QStandardItem("Message"));

	ui.StatusTreeView->setModel(m_StatusViewModel.get());

	ui.StatusTreeView->setColumnWidth(0, 70);
	ui.StatusTreeView->setColumnWidth(1, 150);
	ui.StatusTreeView->setColumnWidth(2, 40);
	ui.StatusTreeView->setColumnWidth(3, 400);
}

//-----------------------------------------

EditorTab* LuaWindow::CurrentEditor() {
	auto *w = ui.tabWidget->currentWidget();
	if (!w)
		return nullptr;
	return static_cast<EditorTab*>(w);
}

//-----------------------------------------

void LuaWindow::OpenEngineScirpt(const QString &path) {
	CreateEditor(path, EditorFileSource::Engine);
}

void LuaWindow::OpenScriptFile(const QString &file) {
	CreateEditor(file, EditorFileSource::RawFile);
}

void LuaWindow::CreateEditor(const QString &path, EditorFileSource mode) {
	EditorTab *tab = nullptr;
	
	for (int i = 0, j = ui.tabWidget->count(); i < j; ++i) {
		auto *wtab = dynamic_cast<EditorTab*>(ui.tabWidget->widget(i));
		if (!wtab)
			continue;
		
		if (wtab->GetName().compare(path) != 0)
			continue;

		tab = wtab;
		break;
	}

	if(!tab) {
		tab = new EditorTab(this, ui.tabWidget, ui.tabWidget, path, mode);
		ui.tabWidget->addTab(tab, tab->GetDisplayName());
		connect(tab, SIGNAL(StateChanged(EditorTab*)), SLOT(ScriptTabStateChanged(EditorTab*)));
		connect(tab, SIGNAL(ContextMenuRequested(EditorTab *, QPoint)), SLOT(TabContextMenuRequested(EditorTab *, QPoint)));
	}

	tab->show();
	ui.tabWidget->setCurrentIndex(ui.tabWidget->indexOf(tab));

	if (!path.isEmpty()) {
		std::string s = path.toUtf8().constData();
		auto &Recent = mgdtSettings::get().Recent;
		Recent.AddOpenedScript(s, mode);
	}
}

//-----------------------------------------

void LuaWindow::OpenScriptAction() {
	QStringList list = QFileDialog::getOpenFileNames(this, "Select a script to open...", QString(), FileFilter_Scripts);
	QStringList::Iterator it = list.begin();
	while(it != list.end()) {
		OpenScriptFile(*it);
		++it;
	}
	RefreshOpenedFiles();
}

void LuaWindow::SaveScriptAction() {
	auto ed = CurrentEditor();
	if (!ed)
		return;
	ed->Save();
}

void LuaWindow::CloseScriptAction() {
	auto ed = CurrentEditor();
	if (!ed)
		return;
	
	if (ed->IsChanged()) {
		auto result = QMessageBox::question(this, "Close Confirmation?",
										   "File is not saved. Do you really want to close it?",
										   QMessageBox::Yes | QMessageBox::No | QMessageBox::Save);

		switch (result) {
		case QMessageBox::Save:
			ed->Save();
		case QMessageBox::Yes:
			break;
		case QMessageBox::No:
		default:
			return;
		}
	}

	ed->CloseEditor();
	RefreshOpenedFiles();
}

void LuaWindow::CloseAllScriptsAction() {

	std::vector<EditorTab*> tabs;

	tabs.reserve(ui.tabWidget->count());
//	auto root = m_OpenedFilesViewModel.get();
	for (int i = 0, j = ui.tabWidget->count(); i < j; ++i) {
		auto *tab = dynamic_cast<EditorTab*>(ui.tabWidget->widget(i));
		if (!tab)
			continue;

		tabs.push_back(tab);
	}

	for(auto ed: tabs) {
		if (ed->IsChanged()) {
			auto result = QMessageBox::question(this, "Close Confirmation?",
												"File is not saved. Do you really want to close it?",
												QMessageBox::Yes | QMessageBox::No | QMessageBox::Save | QMessageBox::Cancel);

			switch (result) {
				case QMessageBox::Save:
					ed->Save();
				case QMessageBox::Yes:
					break;
				case QMessageBox::Cancel:
					RefreshOpenedFiles();
					return;
				case QMessageBox::No:
				default:
					continue;
			}
		}

		ed->CloseEditor();
	}
	RefreshOpenedFiles();
}

void LuaWindow::SendScriptAction() {
	auto ed = CurrentEditor();
	if (!ed)
		return;
	ed->SendText(false);
}

void LuaWindow::SendSelectedScriptAction() {
	auto ed = CurrentEditor();
	if (!ed)
		return;
	ed->SendText(true);
}

void LuaWindow::ReloadAction() {
	auto ed = CurrentEditor();
	if (!ed)
		return;
	if (ed->IsChanged()) {
		auto result = QMessageBox::question(this, "Script is modiffied",
									   "Script is modiffied. Do you want to reload it?",
									   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

		if (result != QMessageBox::Yes)
			return;
	}

	ed->Reload();
}

void LuaWindow::ReloadAllAction() {
	auto result = QMessageBox::question(this, "Confirmation",
										"Do you really want to reload all scripts?",
										QMessageBox::Yes | QMessageBox::No);
	if (result != QMessageBox::Yes)
		return;

	for (int i = 0, j = ui.tabWidget->count(); i < j; ++i) {
		auto *tab = dynamic_cast<EditorTab*>(ui.tabWidget->widget(i));
		if (!tab)
			continue;
		tab->Reload();
	}
}

void LuaWindow::KillScirptsAction() {
	GetRemoteConsole().ExecuteCode(" Inst.ScriptEngine:KillAllScripts() ");
}

void LuaWindow::NewScriptAction() {
	CreateEditor("", EditorFileSource::DummyFile);
}

void LuaWindow::ValidateScriptAction() {
	ValidateScript(CurrentEditor());
}

void LuaWindow::ValidateScript(EditorTab *sender) {
	sender->Validate();
}

//-----------------------------------------

void LuaWindow::RefreshOpenedFiles() {
	ResetModelViews();

	auto root = m_OpenedFilesViewModel.get();
	for (int i = 0, j = ui.tabWidget->count(); i < j; ++i) {
		auto *tab = dynamic_cast<EditorTab*>(ui.tabWidget->widget(i));
		if (!tab)
			continue;

		const char *state_icon;
		const char *type_icon;
		const char *type_name;
		if (tab->IsChanged())
			state_icon = ":/mgdt/icons/red_down_save.png";
		else
			state_icon = ":/mgdt/icons/gray_down_save.png";
		
		switch (tab->GetType()) {
		case EditorFileSource::DummyFile:
			type_icon = ":/mgdt/icons/yellow_note.png";
			type_name = "note";
			break;
		case EditorFileSource::RawFile:
			type_icon = ":/mgdt/icons/blue_file.png";
			type_name = "file";
			break;
		case EditorFileSource::Engine:
			type_icon = ":/mgdt/icons/blue_down_arrow.png";
			type_name = "remote";
			break;
		default:
			type_icon = type_name = "";
			break;
		}

		QList<QStandardItem*> cols;
		QStandardItem *item = new QStandardItem("");
		item->setData(QIcon(type_icon), Qt::DecorationRole);
		item->setData(QVariant::fromValue(tab), Qt::UserRole + 1);
		cols << item;
		cols << new QStandardItem(QIcon(state_icon), "");
		cols << new QStandardItem(tab->GetDisplayName());
		cols << new QStandardItem(type_name);
		root->appendRow(cols);
	}

	root->sort(2);
}

void LuaWindow::OpenedFilesItemClicked(QModelIndex index) {
	int r = index.row();

	auto item = m_OpenedFilesViewModel->invisibleRootItem()->child(r);
	auto tab = item->data(Qt::UserRole + 1).value<EditorTab*>();

	ui.tabWidget->setCurrentIndex(ui.tabWidget->indexOf(tab));
}

void LuaWindow::RefreshEditorStatus(EditorTab *editor) {
	EditorStatusText text;
	for (int i = 0, j = ui.tabWidget->count(); i < j; ++i) {
		auto *tab = dynamic_cast<EditorTab*>(ui.tabWidget->widget(i));
		if (!tab)
			continue;
		tab->GetEditorStatusLines(text);
	}

	ResetStatusModelView();

	auto root = m_StatusViewModel.get();
	for (auto &it : text.Lines) {
		QList<QStandardItem*> cols;
		QStandardItem *item;

		const char *type_str, *type_icon;
		switch (it.Status) {
		case EditorStatusLine::Type::None:
			type_str = "";
			type_icon = "";
			break;
		case EditorStatusLine::Type::Error:
			type_str = "Error";
			type_icon = ":/mgdt/icons/red_flag.png";
			break;
		case EditorStatusLine::Type::Warning:
			type_str = "Warning";
			type_icon = "";
			break;
		case EditorStatusLine::Type::Hint:
			type_str = "Hint";
			type_icon = "";
			break;
		default:
			type_str = "Unknown";
		}

		char line_str[64];
		if (it.SourceLine > 0)
			sprintf_s(line_str, "%d", it.SourceLine);
		else
			line_str[0] = 0;

		cols << (item = new QStandardItem(type_str));
		cols << new QStandardItem(it.Sender->GetDisplayName());
		cols << new QStandardItem(line_str);
		cols << new QStandardItem(it.Message.c_str());
		item->setData(QIcon(type_icon), Qt::DecorationRole);
		item->setData(QVariant::fromValue(it.Sender), Qt::UserRole + 1);
		root->appendRow(cols);
	}
}

void LuaWindow::EditorStatusItemClicked(QModelIndex index) {
	int r = index.row();

	auto item = m_StatusViewModel->invisibleRootItem()->child(r);
	auto tab = item->data(Qt::UserRole + 1).value<EditorTab*>();

	ui.tabWidget->setCurrentIndex(ui.tabWidget->indexOf(tab));
}

//-----------------------------------------

void LuaWindow::ScriptTabStateChanged(EditorTab *tab) {
	auto idx = ui.tabWidget->indexOf(tab);
	if (idx < 0)
		return;

	ui.tabWidget->setTabText(idx, tab->GetDisplayName());

	RefreshOpenedFiles();
	RefreshEditorStatus(tab);
}

//-----------------------------------------

void LuaWindow::TabContextMenuRequested(EditorTab *sender, QPoint pos) {
	QMenu menu(sender);

	if (sender->IsTextSelected())
		menu.addAction(ui.actionSendSelctedText);
	else
		menu.addAction(ui.actionSendScript);

	menu.addSeparator();

	menu.addAction(ui.actionCopy);
	menu.addAction(ui.actionPaste);
	menu.addAction(ui.actionCut);

	//menu.addSeparator();
/*
	QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionCut;
*/

	menu.exec(sender->mapToGlobal(pos));
}

//-----------------------------------------

void LuaWindow::closeEvent( QCloseEvent * event ) {  
	event->ignore();	    

	bool Unsaved = false;
	for (int i = 0, j = ui.tabWidget->count(); i < j; ++i) {
		auto *tab = dynamic_cast<EditorTab*>(ui.tabWidget->widget(i));
		if (!tab)
			continue;

		if (tab->IsChanged())
			Unsaved = true;
	}

	QMessageBox::StandardButton result;
	if (!Unsaved) {
		result = QMessageBox::No;
	} else  {
		result = QMessageBox::question(this, "Close Confirmation?",
										"There are unsaved files. Do you want to save them before exit?", 
										QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	}

	switch (result) {
	case QMessageBox::Yes:
		for (auto *it : ui.tabWidget->children()) {
			auto *tab = static_cast<EditorTab*>(it);
			if (!tab)
				continue;

			if (!tab->IsChanged())
				continue;

			ui.tabWidget->setCurrentIndex(ui.tabWidget->indexOf(tab));

			tab->Save();
			if (tab->IsChanged())
				return;
		}
		//break;
	case QMessageBox::No:
		event->accept();	    
		emit WindowClosed();
		return;

	case QMessageBox::NoButton:
	case QMessageBox::Cancel:
	default:
		return;
	}

}; 
