
#include <qapplication.h>

#include "Windows/MainWindow.h"
#include <Foundation/OS/Path.h>
#include <ToolBase/AppConfig.h>
#include <ToolBase/Module.h>
#include <ToolBase/ModuleRegistration.h>
#include <ToolBase/QtLogSink.h>

#include <OrbitLogger/src/sink/FileSink.h>

using OrbitLogger::LogCollector;
using OrbitLogger::StdFileLoggerSink;

namespace {

MoonGlare::AppConfig::MapType GetAppConig() {
    return {
        {"AppName", "MGPerfView"},
        {"ConfigPath", MoonGlare::OS::GetSettingsDirectory() },
    };
}

}

int main(int argc, char* argv[]) {
    OrbitLogger::ThreadInfo::SetName("MAIN", true);
    LogCollector::Start();
    LogCollector::AddLogSink<StdFileLoggerSink>("logs/MGPerfView.log");

    using MoonGlare::PerfView::MainWindow;

    int r;
    QApplication a(argc, argv);
    qInstallMessageHandler(&MoonGlare::QtLogSink);

    MoonGlare::RegisterBaseModules();

    {
        auto modmgr = MoonGlare::ModuleManager::CreateModuleManager(GetAppConig());
        modmgr->Initialize();
        r = a.exec();
        modmgr->Finalize();
        modmgr.reset();
    }

    LogCollector::Stop();
    return r;
}
