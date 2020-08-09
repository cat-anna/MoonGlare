#include "MainWindow.h"
#include <QtLogSink.h>
#include <os/Path.h>
#include <qapplication.h>
#include <runtime_modules.h>
#include <runtime_modules/app_config.h>
#include <tool_base_module_registration.h>
#include <tool_base_qt_module_registration.h>
// #include <OrbitLogger/src/sink/FileSink.h>

using OrbitLogger::LogCollector;
// using OrbitLogger::StdFileLoggerSink;

namespace {

MoonGlare::Tools::RuntineModules::AppConfig::MapType GetAppConfig() {
    return {
        {"AppName", "MGReconGui"},
        {"ConfigPath", MoonGlare::OS::GetSettingsDirectory()},
    };
}

} // namespace

int main(int argc, char *argv[]) {
    OrbitLogger::ThreadInfo::SetName("MAIN", true);
    LogCollector::Start();
    // LogCollector::AddLogSink<StdFileLoggerSink>("logs/MGReconGui.log");

    int r;
    QApplication a(argc, argv);
    qInstallMessageHandler(&MoonGlare::QtLogSink);

    MoonGlare::Tools::RegisterBaseModules();
    MoonGlare::Tools::RegisterBaseQtModules();

    MoonGlare::Tools::ModuleClassRegister::Register<MoonGlare::Tools::Recon::MainWindow> MainWindowReg("MainWindow");

    {
        auto modmgr = MoonGlare::Tools::ModuleManager::CreateModuleManager(GetAppConfig());
        modmgr->Initialize();
        r = a.exec();
        modmgr->Finalize();
        modmgr.reset();
    }

    LogCollector::Stop();
    return r;
}
