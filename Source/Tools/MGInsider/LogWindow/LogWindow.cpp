#include PCH_HEADER
#include "mgdtSettings.h"
#include "LogWindow.h"
#include "ui_LogWindow.h"
#include "LogWindowBaseTab.h"

LogWindow::LogWindow(QWidget *parent) : SubWindow(parent) {
	ui = new Ui::LogWindow();
	ui->setupUi(this);

//	auto &settings = mgdtSettings::get();
//	settings.Window.LogWindow.Apply(this);

	LogWindowTabRegister::GetRegister()->Enumerate([this](const LogWindowTabRegister::ClassRegister::ClassInfo &ci) {
		auto ptr = ci.Create(this);
		ui->tabWidget->addTab(ptr, ptr->GetName());
		//if (LastTab == ptr->GetName().toUtf8().constData()) {
			////ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ptr));
		//}
	});
	ui->tabWidget->setCurrentIndex(0);
}

LogWindow::~LogWindow(){
//	auto &settings = mgdtSettings::get();
//	settings.Window.LogWindow.Store(this);
	//auto tab = CurrentEditor();
	//if (tab)
		//settings.LogWindow.LastTab = tab->GetName().toUtf8().constData();
	delete ui;
}

//-----------------------------------------

LogWindowBaseTab* LogWindow::CurrentEditor() {
	auto *w = ui->tabWidget->currentWidget();
	if (!w)
		return nullptr;
	return static_cast<LogWindowBaseTab*>(w);
}

//-----------------------------------------
