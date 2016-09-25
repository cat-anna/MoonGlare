#ifndef MAINFORM_H
#define MAINFORM_H

#include <DockWindowInfo.h>
#include <Notifications.h>
#include <qtUtils.h>
#include <iEditor.h>
#include <ChangesManager.h>

namespace Ui { class MainWindow; }

namespace MoonGlare {
namespace Editor {

namespace Module {
	class DataModule;
}
class FileSystem;
using SharedFileSystem = std::shared_ptr<FileSystem>;

struct SharedData {
	std::unordered_map<std::string, std::string> m_FileIconMap;
	std::unique_ptr<QtShared::ChangesManager> m_ChangesManager;

	SharedData() {
		m_ChangesManager = std::make_unique<QtShared::ChangesManager>();
	}
};

class MainWindow 
	: public QMainWindow
	, public QtShared::UserQuestions
	, public QtShared::iSettingsUser {
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();
	static MainWindow* Get();

	SharedFileSystem GetFilesystem() { return m_FileSystem; }
	SharedData* GetSharedData() { return &m_SharedData; }

protected:
	void closeEvent(QCloseEvent * event);
	void showEvent(QShowEvent * event);

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;
private:
	std::unique_ptr<Ui::MainWindow> m_Ui;
	std::vector<QtShared::SharedDockWindowInfo> m_DockWindows;
	std::unordered_map<std::string, QtShared::SharedDockWindowInfo> m_Editors;
	SharedData m_SharedData;

	Module::SharedDataModule m_DataModule;
	SharedFileSystem m_FileSystem;
	void NewModule(const std::string& MasterFile);
	void OpenModule(const std::string& MasterFile);
	void CloseModule();
public slots:
	void OpenFileEditor(const std::string& FileURI);
protected slots:
	void NewModuleAction();
	void OpenModuleAction();
	void CloseModuleAction();
	void CloseEditorAction();
};

} //namespace Editor
} //namespace MoonGlare

#endif // MAINFORM_H
