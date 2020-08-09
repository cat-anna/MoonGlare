#pragma once

#include <qevent.h>
#include <qmainwindow.h>
#include <recon_client.h>
#include <runtime_modules.h>
#include <runtime_modules/settings_user.h>

namespace Ui {
class MainWindow;
}

namespace MoonGlare::Tools::Recon {

class MainWindow : public QMainWindow, public iModule, public RuntineModules::iSettingsUser {
    Q_OBJECT
public:
    MainWindow(SharedModuleManager modmgr);
    ~MainWindow();

    bool DoSaveSettings(pugi::xml_node node) const override;
    bool DoLoadSettings(const pugi::xml_node node) override;

    bool Initialize() override;
    bool PostInit() override;
    bool Finalize() override;

protected:
    std::unique_ptr<Ui::MainWindow> ui;
    std::shared_ptr<Tools::RemoteConsole::ReconClient> reconClient;

    void Send(const QString &text, bool addToHistory);
    void RemoveAll(const QString &text);

    void Save();
    void Load();

    std::string historyFileName;
};

} // namespace MoonGlare::Tools::Recon
