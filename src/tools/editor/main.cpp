#include <QApplication>
#include <orbit_logger.h>
#include <orbit_logger/sink/file_sink.h>
#include <os/Path.h>
#include <qt_log_sink.h>
#include <runtime_modules.h>
#include <runtime_modules/app_config.h>
#include <tool_base_module_registration.h>
#include <tool_base_qt_module_registration.h>

using OrbitLogger::LogCollector;
using OrbitLogger::StdFileLoggerSink;

namespace {

MoonGlare::Tools::RuntineModules::AppConfig::MapType GetAppConfig() {
    return {
        {"AppName", "editor"},
        {"ConfigPath", MoonGlare::OS::GetSettingsDirectory()},
    };
}

} // namespace

int main(int argc, char *argv[]) {
    OrbitLogger::ThreadInfo::SetName("MAIN", true);
    LogCollector::Start();
    LogCollector::AddLogSink<StdFileLoggerSink>("logs/editor.log");

    int r;
    QApplication a(argc, argv);
    qInstallMessageHandler(&MoonGlare::Tools::QtLogSink);

    MoonGlare::Tools::RegisterBaseModules();
    MoonGlare::Tools::RegisterBaseQtModules();

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
