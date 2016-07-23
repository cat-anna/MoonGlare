#ifndef MAINFORM_H
#define MAINFORM_H

#include <DockWindowInfo.h>
#include <EditorSettings.x2c.h>
#include <Notifications.h>

namespace Ui { class MainWindow; }

namespace MoonGlare {
namespace Editor {

namespace Module {
	class DataModule;
}
class FileSystem;
using SharedFileSystem = std::shared_ptr<FileSystem>;

class MainWindow 
	: public QMainWindow
	, public QtShared::iSettingsUser {
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();
	static MainWindow* Get();

	SharedFileSystem GetFilesystem() { return m_FileSystem; }

protected:
	void closeEvent(QCloseEvent * event);
	void showEvent(QShowEvent * event);

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;
private:
	std::unique_ptr<Ui::MainWindow> m_Ui;
	std::vector<QtShared::SharedDockWindowInfo> m_DockWindows;

	Module::SharedDataModule m_DataModule;
	SharedFileSystem m_FileSystem;
	x2c::Settings::EditorSettings_t m_Settings;
	void NewModule(const std::string& MasterFile);
	void OpenModule(const std::string& MasterFile);
	void CloseModule();
public slots:

protected slots:
	void NewModuleAction();
	void OpenModuleAction();
	void CloseModuleAction();
	void CloseEditorAction();
};

} //namespace Editor
} //namespace MoonGlare

#endif // MAINFORM_H
