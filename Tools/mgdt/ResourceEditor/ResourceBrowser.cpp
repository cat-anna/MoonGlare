#include PCH_HEADER
#include "mgdtSettings.h"
#include "ResourceBrowser.h"
#include "ui_ResourceBrowser.h"
#include "ResourceEditorBaseTab.h"

ResourceBrowser::ResourceBrowser(QWidget *parent)
	: SubWindow(parent)
{
	ui = new Ui::ResourceBrowser();
	ui->setupUi(this);

	connect(ui->actionRefresh, SIGNAL(triggered()), SLOT(Refresh()));

	auto &settings = mgdtSettings::get();
	settings.Window.ResourceBrowser.Apply(this);

	auto &LastTab = settings.ResourceBrowser.LastTab;
	ResourceEditorTabRegister::GetRegister()->Enumerate([this, &LastTab](const ResourceEditorTabRegister::ClassRegister::ClassInfo &ci) {
		auto ptr = ci.Create(this);
		ui->tabWidget->addTab(ptr, ptr->GetName());
		if (LastTab == ptr->GetName().toUtf8().constData()) {
			ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ptr));
		}
	});
}

ResourceBrowser::~ResourceBrowser(){
	auto &settings = mgdtSettings::get();
	settings.Window.ResourceBrowser.Store(this);
	auto tab = CurrentEditor();
	if (tab)
		settings.ResourceBrowser.LastTab = tab->GetName().toUtf8().constData();
	delete ui;
}

//-----------------------------------------

ResourceEditorBaseTab* ResourceBrowser::CurrentEditor() {
	auto *w = ui->tabWidget->currentWidget();
	if (!w)
		return nullptr;
	return static_cast<ResourceEditorBaseTab*>(w);
}

//-----------------------------------------

void ResourceBrowser::Refresh() {
	auto e = CurrentEditor();
	e->Refresh();
}
