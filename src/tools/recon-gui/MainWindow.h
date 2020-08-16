#pragma once

#include <qevent.h>
#include <qmainwindow.h>
#include <recon_client.h>
#include <runtime_modules.h>
#include <runtime_modules/widget_settings_provider.hpp>

namespace Ui {
class MainWindow;
}

namespace MoonGlare::Tools::Recon {

class MainWindow : public QMainWindow, public iModule, public RuntineModules::iWidgetSettingsProvider {
    Q_OBJECT
public:
    MainWindow(SharedModuleManager modmgr);
    ~MainWindow();

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
